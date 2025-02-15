
#include "fornani/gui/Console.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::gui {

Console::Console(automa::ServiceProvider& svc)
	: portrait(svc), nani_portrait(svc, false), m_services(&svc), item_widget(svc), m_nineslice(svc, corner_factor, edge_factor), m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "standard", 64} {
	origin = {pad, svc.constants.screen_dimensions.y - pad_y};
	text_suite = svc.text.console;
	set_texture(svc.assets.t_ui);

	dimensions = sf::Vector2<float>{svc.constants.f_screen_dimensions.x - 2 * pad, svc.constants.f_screen_dimensions.y / height_factor};
	position = sf::Vector2<float>{svc.constants.f_center_screen.x, origin.y - dimensions.y * 0.5f};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};
}

void Console::begin() {
	flags.set(ConsoleFlags::active);
	m_path.set_section("open");
	m_services->controller_map.set_action_set(config::ActionSet::Menu);
	NANI_LOG_INFO(m_logger, "Console began.");
}

void Console::update(automa::ServiceProvider& svc) {
	handle_inputs(svc.controller_map);
	if (!writer) { return; }
	if (active()) { m_path.update(); }
	if (writer->writing()) { svc.soundboard.flags.console.set(audio::Console::speech); }
	position = m_path.get_position();
	dimensions = m_path.get_dimensions();
	m_nineslice.direct_update(svc, m_path.get_position(), m_path.get_dimensions(), corner_factor, edge_factor);
	writer->set_bounds(sf::Vector2<float>{position.x + dimensions.x * 0.5f - 2.f * border.left, position.y + dimensions.y * 0.5f - border.top});
	writer->set_position(position + sf::Vector2<float>{border.left, border.top} - dimensions * 0.5f);
	if (m_path.finished()) { flags.set(ConsoleFlags::extended); }
	writer->selection_mode() ? flags.set(ConsoleFlags::selection_mode) : flags.reset(ConsoleFlags::selection_mode);
	writer->update();

	if (flags.test(ConsoleFlags::active)) {
		portrait.update(svc);
		nani_portrait.update(svc);
		if (flags.test(ConsoleFlags::display_item)) { item_widget.update(svc); }
		if (writer->response_triggered()) {
			nani_portrait.reset(*m_services);
			writer->reset_response();
		}
	}
}

void Console::render(sf::RenderWindow& win) {
	if (!writer) { return; }
	m_nineslice.render(win);
	if (flags.test(ConsoleFlags::display_item)) { item_widget.render(*m_services, win); }
	if (flags.test(ConsoleFlags::portrait_included)) {
		portrait.render(win);
		writer->responding() ? nani_portrait.bring_in() : nani_portrait.send_out();
		nani_portrait.render(win);
	}
}

void Console::set_source(dj::Json& json) { text_suite = json; }

void Console::set_texture(sf::Texture& tex) { m_nineslice.set_texture(tex); }

void Console::load_and_launch(std::string_view key, OutputType type) {
	m_output_type = type;
	flags.set(ConsoleFlags::active);
	writer = writer ? std::move(std::make_unique<TextWriter>(*m_services, text_suite, key)) : std::make_unique<TextWriter>(*m_services, text_suite, key);
	update(*m_services);
	native_key = key;
	portrait.reset(*m_services);
	nani_portrait.reset(*m_services);
	item_widget.reset(*m_services);
	begin();
}

void Console::load_single_message(std::string_view message) { writer = writer ? std::move(std::make_unique<TextWriter>(*m_services, message)) : std::make_unique<TextWriter>(*m_services, message); }

void Console::display_item(int item_id) {
	flags.set(ConsoleFlags::display_item);
	item_widget.set_id(item_id);
}

void Console::display_gun(int gun_id) {
	flags.set(ConsoleFlags::display_item);
	item_widget.set_id(gun_id, true);
}

void Console::write(sf::RenderWindow& win, bool instant) {
	if (!flags.test(ConsoleFlags::active)) { return; }
	if (!extended()) { return; }
	if (!writer) { return; }
	instant ? writer->write_instant_message(win) : writer->write_gradual_message(win);
	writer->write_responses(win);
}

void Console::write(sf::RenderWindow& win) { write(win, m_output_type == OutputType::instant); }

void Console::append(std::string_view key) { writer->append(key); }

void Console::end() {
	writer->flush();
	flags.reset(ConsoleFlags::active);
	flags.reset(ConsoleFlags::portrait_included);
	flags.reset(ConsoleFlags::extended);
	flags.reset(ConsoleFlags::display_item);
	flags.set(ConsoleFlags::off_trigger);
	m_path.reset();
	m_services->controller_map.set_action_set(config::ActionSet::Platformer);
	NANI_LOG_INFO(m_logger, "Console ended.");
	writer = {};
}

void Console::clean_off_trigger() { flags.reset(ConsoleFlags::off_trigger); }

void Console::end_tick() { clean_off_trigger(); }

void Console::include_portrait(int id) {
	flags.set(ConsoleFlags::portrait_included);
	portrait.set_id(id);
}

std::string Console::get_key() { return native_key; }

void Console::handle_inputs(config::ControllerMap& controller) {
	if (!writer) { return; }
	auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).triggered;
	auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).triggered;
	auto const& canceled = controller.digital_action_status(config::DigitalAction::menu_cancel).triggered;
	auto const& next = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
	auto const& exit = controller.digital_action_status(config::DigitalAction::menu_cancel).triggered;
	auto const& skip = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
	auto const& skip_released = controller.digital_action_status(config::DigitalAction::menu_select).released;
	static bool finished{};

	if (skip) {
		if (writer->writing() && writer->can_skip()) { writer->skip_ahead(); }
	}
	if (skip_released) { writer->enable_skip(); }
	if (next && !writer->delaying()) {
		if (!writer->writing()) { m_services->soundboard.flags.console.set(audio::Console::next); }
		finished = writer->request_next();
		writer->reset_delay();
	}
	if (exit || finished) {
		if (writer->complete()) {
			m_services->soundboard.flags.console.set(audio::Console::done);
			writer->shutdown();
			end();
		}
	}
}

} // namespace fornani::gui
