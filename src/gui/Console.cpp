
#include "fornani/gui/Console.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"

namespace gui {

Console::Console(automa::ServiceProvider& svc) : portrait(svc), nani_portrait(svc, false), writer(svc), m_services(&svc), item_widget(svc), sprite(svc, corner_factor, edge_factor) {
	origin = {pad, svc.constants.screen_dimensions.y - pad_y};
	text_suite = svc.text.console;
	set_texture(svc.assets.t_ui);

	dimensions = sf::Vector2<float>{(float)svc.constants.screen_dimensions.x - 2 * pad, (float)svc.constants.screen_dimensions.y / height_factor};
	position = sf::Vector2<float>{svc.constants.f_center_screen.x, origin.y - dimensions.y * 0.5f};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};
}

void Console::begin() {
	flags.set(ConsoleFlags::active);
	writer.start();
	sprite.start(*m_services, position);
}

void Console::update(automa::ServiceProvider& svc) {
	sprite.update(svc, position, dimensions, corner_factor, edge_factor);
	writer.set_bounds(sf::Vector2<float>{position.x + dimensions.x * 0.5f - 2.f * border.left, position.y + dimensions.y * 0.5f - border.top});
	writer.set_position(position + sf::Vector2<float>{border.left, border.top} - dimensions * 0.5f);
	if (sprite.is_extended()) { flags.set(ConsoleFlags::extended); }
	writer.selection_mode() ? flags.set(ConsoleFlags::selection_mode) : flags.reset(ConsoleFlags::selection_mode);
	writer.update();

	if (flags.test(ConsoleFlags::active)) {
		portrait.update(svc);
		nani_portrait.update(svc);
		if (flags.test(ConsoleFlags::display_item)) { item_widget.update(svc); }
		if (writer.response_triggered()) {
			nani_portrait.reset(*m_services);
			writer.reset_response();
		}
	}
}

void Console::render(sf::RenderWindow& win) {
	sprite.render(win);
	if (flags.test(ConsoleFlags::display_item)) { item_widget.render(*m_services, win); }
	if (flags.test(ConsoleFlags::portrait_included)) {
		portrait.render(win);
		writer.responding() ? nani_portrait.bring_in() : nani_portrait.send_out();
		nani_portrait.render(win);
	}
}

void Console::set_source(dj::Json& json) { text_suite = json; }

void Console::set_texture(sf::Texture& tex) { sprite.set_texture(tex); }

void Console::load_and_launch(std::string_view key) {
	if (!flags.test(ConsoleFlags::loaded)) {
		native_key = key;
		writer.load_message(text_suite, key);
		portrait.reset(*m_services);
		nani_portrait.reset(*m_services);
		item_widget.reset(*m_services);
		flags.set(ConsoleFlags::loaded);
		begin();
	}
}

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
	instant ? writer.write_instant_message(win) : writer.write_gradual_message(win);
	writer.write_responses(win);
}

void Console::append(std::string_view key) { writer.append(key); }

void Console::end() {
	writer.flush_communicators();
	flags.reset(ConsoleFlags::active);
	flags.reset(ConsoleFlags::portrait_included);
	flags.reset(ConsoleFlags::extended);
	flags.reset(ConsoleFlags::display_item);
	flags.set(ConsoleFlags::off_trigger);
	sprite.end();
}

void Console::clean_off_trigger() { flags.reset(ConsoleFlags::off_trigger); }

void Console::end_tick() {
	if (!flags.test(ConsoleFlags::active)) { flags.reset(ConsoleFlags::loaded); }
	clean_off_trigger();
}

void Console::include_portrait(int id) {
	flags.set(ConsoleFlags::portrait_included);
	portrait.set_id(id);
}

std::string Console::get_key() { return native_key; }

} // namespace gui
