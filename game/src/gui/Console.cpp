
#include "fornani/gui/Console.hpp"

#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::gui {

Console::Console(automa::ServiceProvider& svc)
	: m_npc_portrait(svc), m_nani_portrait(svc, false), m_services(&svc), m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "standard", 64}, m_styling{.corner_factor{28}, .edge_factor{1}, .padding_scale{1.1f}},
	  m_nineslice(svc, svc.assets.get_texture("blue_console"), {m_styling.corner_factor, m_styling.corner_factor}, {m_styling.edge_factor, m_styling.edge_factor}), m_mode{ConsoleMode::writing}, m_response_offset{-48.f, 16.f},
	  m_exit_stall{650} {
	text_suite = svc.text.console;
	m_path.set_section("open");
	m_began = true;
	m_path.update();
	m_position = m_path.get_position();
}

Console::Console(automa::ServiceProvider& svc, std::string_view message) : Console(svc) { load_single_message(message); }

Console::Console(automa::ServiceProvider& svc, dj::Json& source, std::string_view key, OutputType type) : Console(svc) {
	if (type == OutputType::no_skip) { m_exit_stall.start(); }
	set_source(source);
	load_and_launch(key, type);
}

void Console::update(automa::ServiceProvider& svc) {
	m_began = false;
	m_exit_stall.update();
	if (!is_active()) { return; }
	if (!m_writer) { return; }
	handle_inputs(svc.controller_map);
	if (is_active()) { m_path.update(); }
	if (m_path.finished() && m_writer) {
		if (m_writer->is_stalling()) { m_writer->start(); }
	}
	if (!m_writer) { return; }
	if (m_writer->is_writing()) { svc.soundboard.flags.console.set(audio::Console::speech); }
	m_position = m_path.get_position();
	m_dimensions = m_path.get_dimensions();
	m_nineslice.set_offset(-m_path.get_local_center());
	m_nineslice.set_position(m_path.get_position());
	m_nineslice.set_dimensions(m_path.get_dimensions());
	auto vert_factor{1.5f};
	auto to_dim{m_dimensions.componentWiseMul({m_styling.padding_scale, m_styling.padding_scale * vert_factor})};
	m_writer->set_bounds(sf::FloatRect{m_position - to_dim * 0.5f, to_dim});
	m_writer->update();
	if (m_response) {
		m_response->set_position(m_position + m_response_offset);
		m_response->update();
	}
	m_npc_portrait.update(svc);
	m_nani_portrait.update(svc);
	if (m_item_widget) { m_item_widget->update(svc); }
	// check for response
	for (auto& code : m_codes) {
		if (code.set == m_writer->get_current_suite_set() && code.index == m_writer->get_index()) {
			if (code.is_response() && m_writer->is_available()) { m_writer->respond(); }
		}
	}
}

void Console::render(sf::RenderWindow& win) {
	// debug();
	if (!m_writer || !is_active()) { return; }
	m_nineslice.render(win);
	if (m_item_widget) { m_item_widget->render(*m_services, win); }
	if (m_flags.test(ConsoleFlags::portrait_included)) {
		m_npc_portrait.render(win);
		m_mode == ConsoleMode::responding ? m_nani_portrait.bring_in() : m_nani_portrait.send_out();
		m_nani_portrait.render(win);
	}
	if (m_response) { m_response->render(win); }
	// m_writer->debug();
}

void Console::set_source(dj::Json& json) { text_suite = json; }

void Console::set_nani_sprite(sf::Sprite const& sprite) { m_nani_portrait.set_texture(sprite.getTexture()); }

void Console::handle_actions(int value) {
	switch (value) {
	case 1: // return to main menu
		m_services->state_controller.actions.set(automa::Actions::main_menu);
		m_services->state_controller.actions.set(automa::Actions::trigger);
		break;
	case 2: // restart save
		m_services->state_controller.actions.set(automa::Actions::retry);
		break;
	case 3: m_services->state_controller.actions.set(automa::Actions::delete_file); break;
	}
}

void Console::load_and_launch(std::string_view key, OutputType type) {
	m_writer = std::make_unique<TextWriter>(*m_services, text_suite, key);
	// load message codes
	auto& in_data = text_suite[key]["codes"];
	if (in_data) {
		for (auto& code : in_data.as_array()) {
			if (code.as_array().size() < 5) { NANI_LOG_ERROR(m_logger, "Invalid Text Json data, too few codes were read!"); }
			auto in_ints = std::vector<int>{};
			for (auto& input : code.as_array()) { in_ints.push_back(input.as<int>()); }
			auto in_code = MessageCode{static_cast<CodeSource>(in_ints[0]), in_ints[1], in_ints[2], static_cast<MessageCodeType>(in_ints[3]), in_ints[4]};
			// read extra values, if they exist
			if (code.as_array().size() > 5) {
				in_code.extras = std::vector<int>{};
				for (auto i{5}; i < code.as_array().size(); ++i) { in_code.extras->push_back(in_ints[i]); }
			}
			m_codes.push_back(in_code);
		}
	}
	m_output_type = type;
	native_key = key;
	m_npc_portrait.reset(*m_services);
	m_nani_portrait.reset(*m_services);
}

void Console::load_single_message(std::string_view message) { m_writer = std::make_unique<TextWriter>(*m_services, message); }

void Console::display_item(int item_id) { m_item_widget = ItemWidget(*m_services, ItemWidgetType::item, item_id); }

void Console::display_gun(int gun_id) { m_item_widget = ItemWidget(*m_services, ItemWidgetType::gun, gun_id); }

void Console::write(sf::RenderWindow& win, bool instant) {
	if (!is_active()) { return; }
	if (!m_writer) { return; }
	instant ? m_writer->write_instant_message(win) : m_writer->write_gradual_message(win);
}

void Console::write(sf::RenderWindow& win) { write(win, m_output_type == OutputType::instant); }

void Console::append(std::string_view key) { m_writer->append(key); }

void Console::end() {
	m_services->soundboard.flags.console.set(audio::Console::done);
	m_mode = ConsoleMode::off;
}

void Console::include_portrait(int id) {
	m_flags.set(ConsoleFlags::portrait_included);
	m_npc_portrait.set_id(id);
}

std::string Console::get_key() const { return native_key; }

void Console::handle_inputs(config::ControllerMap& controller) {
	if (m_exit_stall.running()) { return; }
	auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).triggered;
	auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).triggered;
	auto const& next = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
	auto const& exit = controller.digital_action_status(config::DigitalAction::menu_cancel).triggered;
	auto const& skip = controller.digital_action_status(config::DigitalAction::menu_select).held;
	auto const& released = controller.digital_action_status(config::DigitalAction::menu_select).released;
	static bool finished{};
	static bool can_skip{};
	bool responded{};

	// check for exit
	if (exit && m_output_type != OutputType::no_skip) {
		end();
		return;
	}

	// in response mode
	if (m_response) {
		if (m_response->handle_inputs(controller, m_services->soundboard)) {
			return;
		} else {
			// do something with response selection
			// set target suite if response code demands it
			if (get_response_code(m_response->get_selection()).is_suite_return()) { m_writer->set_suite(get_response_code(m_response->get_selection()).value); }
			if (get_response_code(m_response->get_selection()).is_action()) { handle_actions(get_response_code(m_response->get_selection()).value); }
			if (get_response_code(m_response->get_selection()).is_exit()) {
				end();
				return;
			}
			m_response = {};
			responded = true;
		}
	}

	// create response dialog
	if (next && m_writer->is_responding() && !responded) {
		// create a response dialog, feed it inputs, and await its closure before resuming m_writer
		m_response = ResponseDialog(m_services->text, text_suite, native_key, get_message_code().value, m_position + m_response_offset);
		m_mode = ConsoleMode::responding;
		m_writer->wait();
		m_services->soundboard.flags.console.set(audio::Console::next);
		return;
	}

	// go to next message or exit
	if ((next && m_writer->is_ready()) || responded) {
		auto code = get_message_code();
		if (code.is_redirect() && !responded) {
			m_writer->set_suite(code.value);
			if (code.extras) { m_writer->set_index(code.extras.value()[0]); }
		}
		m_services->soundboard.flags.console.set(audio::Console::next);
		finished = m_writer->request_next();
		can_skip = false;
	}

	// speed up text
	if (released) { can_skip = true; }
	if (m_writer->is_stalling()) { can_skip = false; }
	if (m_output_type == OutputType::no_skip) { can_skip = false; }
	(skip && can_skip) ? m_writer->speed_up() : m_writer->slow_down();

	if (finished) {
		if (m_writer->exit_requested()) { end(); }
	}
}

void Console::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Console Debug")) {
		if (m_response) {
			ImGui::Text("Response Selection: %i", m_response->get_selection());
			get_response_code(m_response->get_selection()).debug();
			ImGui::Separator();
		}
		if (m_writer) { get_message_code().debug(); }
		ImGui::End();
	}
}

auto Console::get_message_code() const -> MessageCode {
	for (auto& code : m_codes) {
		if (code.index == m_writer->get_index() && code.source == CodeSource::suite && code.set == m_writer->get_current_suite_set()) { return code; }
	}
	return MessageCode();
}

auto Console::get_response_code(int which) const -> MessageCode {
	if (!m_response) { return m_codes.back(); }
	for (auto& code : m_codes) {
		if (code.index == which && code.source == CodeSource::response && code.set == m_response->get_index()) { return code; }
	}
	return MessageCode();
}

void MessageCode::debug() {
	ImGui::Text("Source: %s", source == CodeSource::suite ? "suite" : "response");
	ImGui::Text("Index: %i", index);
	ImGui::Text("Type: %i", static_cast<int>(type));
	ImGui::Text("Value: %i", value);
	if (extras) {
		for (auto& extra : *extras) { ImGui::Text("Extra: %i", extra); }
	}
	ImGui::Separator();
}

} // namespace fornani::gui
