
#include "fornani/gui/Console.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::gui {

Console::Console(automa::ServiceProvider& svc) : Console(svc, "blue_console") {}

Console::Console(automa::ServiceProvider& svc, std::string const& texture_lookup)
	: portrait(svc), nani_portrait(svc, false), m_services(&svc), item_widget(svc), m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "standard", 64},
	  m_styling{.corner_factor{28}, .edge_factor{1}, .padding_scale{1.1f}}, m_nineslice(svc, svc.assets.get_texture(texture_lookup), {m_styling.corner_factor, m_styling.corner_factor}, {m_styling.edge_factor, m_styling.edge_factor}),
	  m_mode{ConsoleMode::off} {
	text_suite = svc.text.console;
}

void Console::begin() {
	m_mode = ConsoleMode::writing;
	m_path.set_section("open");
	NANI_LOG_INFO(m_logger, "Console began.");
}

void Console::update(automa::ServiceProvider& svc) {
	if (!is_active()) { return; }
	if (!writer) { return; }
	handle_inputs(svc.controller_map);
	if (is_active()) { m_path.update(); }
	if (m_path.finished() && writer) {
		if (writer->is_stalling()) { writer->start(); }
	}
	if (!writer) { return; }
	if (writer->is_writing()) { svc.soundboard.flags.console.set(audio::Console::speech); }
	position = m_path.get_position();
	dimensions = m_path.get_dimensions();
	m_nineslice.set_offset(-m_path.get_local_center());
	m_nineslice.set_position(m_path.get_position());
	m_nineslice.set_dimensions(m_path.get_dimensions());
	auto vert_factor{1.5f};
	auto to_dim{dimensions.componentWiseMul({m_styling.padding_scale, m_styling.padding_scale * vert_factor})};
	writer->set_bounds(sf::FloatRect{position - to_dim * 0.5f, to_dim});
	writer->update();
	portrait.update(svc);
	nani_portrait.update(svc);
	if (flags.test(ConsoleFlags::display_item)) { item_widget.update(svc); }
}

void Console::render(sf::RenderWindow& win) {
	if (!writer || !is_active()) { return; }
	m_nineslice.render(win);
	if (flags.test(ConsoleFlags::display_item)) { item_widget.render(*m_services, win); }
	if (flags.test(ConsoleFlags::portrait_included)) {
		portrait.render(win);
		// writer->responding() ? nani_portrait.bring_in() : nani_portrait.send_out();
		nani_portrait.render(win);
	}
}

void Console::set_source(dj::Json& json) { text_suite = json; }

void Console::load_and_launch(std::string_view key, OutputType type) {
	NANI_LOG_INFO(m_logger, "Loading...");
	writer = std::make_unique<TextWriter>(*m_services, text_suite, key);
	NANI_LOG_INFO(m_logger, "Writer created...");
	m_output_type = type;
	native_key = key;
	portrait.reset(*m_services);
	nani_portrait.reset(*m_services);
	item_widget.reset(*m_services);
	begin();
}

void Console::load_single_message(std::string_view message) { writer = std::make_unique<TextWriter>(*m_services, message); }

void Console::display_item(int item_id) {
	flags.set(ConsoleFlags::display_item);
	item_widget.set_id(item_id);
}

void Console::display_gun(int gun_id) {
	flags.set(ConsoleFlags::display_item);
	item_widget.set_id(gun_id, true);
}

void Console::write(sf::RenderWindow& win, bool instant) {
	if (!is_active()) { return; }
	if (!writer) { return; }
	instant ? writer->write_instant_message(win) : writer->write_gradual_message(win);
}

void Console::write(sf::RenderWindow& win) { write(win, m_output_type == OutputType::instant); }

void Console::append(std::string_view key) { writer->append(key); }

void Console::end() {
	writer->flush();
	m_mode = ConsoleMode::off;
	m_path.reset();
	writer = {};
	NANI_LOG_INFO(m_logger, "Console ended.");
}

void Console::include_portrait(int id) {
	flags.set(ConsoleFlags::portrait_included);
	portrait.set_id(id);
}

std::string Console::get_key() { return native_key; }

void Console::handle_inputs(config::ControllerMap& controller) {
	auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).triggered;
	auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).triggered;
	auto const& canceled = controller.digital_action_status(config::DigitalAction::menu_cancel).triggered;
	auto const& next = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
	auto const& exit = controller.digital_action_status(config::DigitalAction::menu_cancel).triggered;
	auto const& skip = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
	auto const& skip_released = controller.digital_action_status(config::DigitalAction::menu_select).released;
	static bool finished{};

	if (skip && writer->is_ready()) { writer->speed_up(); }
	if (skip_released) { writer->slow_down(); }
	if (next && writer->is_ready()) {
		m_services->soundboard.flags.console.set(audio::Console::next);
		finished = writer->request_next();
		NANI_LOG_INFO(m_logger, "Requested next: {}", finished);
	}
	if (exit || finished) {
		if (writer->exit_requested()) {
			m_services->soundboard.flags.console.set(audio::Console::done);
			writer->shutdown();
			end();
		}
	}
}

} // namespace fornani::gui
