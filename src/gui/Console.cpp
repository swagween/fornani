
#include "Console.hpp"
#include <algorithm>
#include "../service/ServiceProvider.hpp"

namespace gui {

Console::Console(automa::ServiceProvider& svc) : portrait(svc), nani_portrait(svc, false), writer(svc), m_services(&svc) {

	text_suite = svc.text.console;

	for (auto& sprite : sprites) { sprite.setTexture(svc.assets.t_ui); }
	sprites.at(0).setTextureRect(sf::IntRect{{0, 0}, {corner_factor, corner_factor}});
	sprites.at(1).setTextureRect(sf::IntRect{{corner_factor, 0}, {edge_factor, corner_factor}});
	sprites.at(2).setTextureRect(sf::IntRect{{corner_factor + edge_factor, 0}, {corner_factor, corner_factor}});
	sprites.at(3).setTextureRect(sf::IntRect{{0, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(4).setTextureRect(sf::IntRect{{corner_factor, corner_factor}, {edge_factor, edge_factor}});
	sprites.at(5).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(6).setTextureRect(sf::IntRect{{0, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprites.at(7).setTextureRect(sf::IntRect{{corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor}});
	sprites.at(8).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor}});

	final_dimensions = sf::Vector2<float>{(float)svc.constants.screen_dimensions.x - 2 * pad, (float)svc.constants.screen_dimensions.y / height_factor};
	current_dimensions.x = final_dimensions.x;
	position = sf::Vector2<float>{origin.x, origin.y - final_dimensions.y};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};

	extent = corner_factor * 2;
}

void Console::begin() {
	current_dimensions.y = corner_factor * 2;
	flags.set(ConsoleFlags::active);
	writer.start();
}

void Console::update(automa::ServiceProvider& svc) {
	writer.set_bounds(position + sf::Vector2<float>{final_dimensions.x - 2 * border.left, final_dimensions.y - 2 * border.top});
	writer.set_position(position + sf::Vector2<float>{border.left, border.top});
	if (flags.test(ConsoleFlags::active)) { extent += speed; }
	extent = std::clamp((float)extent, 0.f, final_dimensions.y);
	current_dimensions.y = extent;
	nine_slice(corner_factor, edge_factor);
	writer.selection_mode() ? flags.set(ConsoleFlags::selection_mode) : flags.reset(ConsoleFlags::selection_mode);
	writer.update();
	if (flags.test(ConsoleFlags::active)) {
		portrait.update(svc);
		nani_portrait.update(svc);
		if (writer.response_triggered()) {
			nani_portrait.reset(*m_services);
			writer.reset_response();
		}
	}
}

void Console::render(sf::RenderWindow& win) {
	for (auto& sprite : sprites) { win.draw(sprite); }
	if (flags.test(ConsoleFlags::portrait_included)) { portrait.render(win); }
	flags.test(ConsoleFlags::portrait_included) && writer.responding() ? nani_portrait.bring_in() : nani_portrait.send_out();
	nani_portrait.render(win);
}

void Console::set_source(dj::Json& json) { text_suite = json; }

void Console::load_and_launch(std::string_view key) {
	if (!flags.test(ConsoleFlags::loaded)) {
		writer.load_message(text_suite, key);
		portrait.reset(*m_services);
		nani_portrait.reset(*m_services);
		flags.set(ConsoleFlags::loaded);
		begin();
	}
}

void Console::write(sf::RenderWindow& win, bool instant) {
	if (!flags.test(ConsoleFlags::active)) { return; }
	instant ? writer.write_instant_message(win) : writer.write_gradual_message(win);
	writer.write_responses(win);
}

void Console::end() {
	extent = current_dimensions.y = corner_factor * 2;
	flags.reset(ConsoleFlags::active);
	flags.reset(ConsoleFlags::loaded);
	flags.reset(ConsoleFlags::portrait_included);
}

void Console::include_portrait(int id) {
	flags.set(ConsoleFlags::portrait_included);
	portrait.set_id(id);
}

void Console::nine_slice(int corner_dim, int edge_dim) {

	// set sizes for stretched 9-slice sprites
	sprites.at(1).setScale({(current_dimensions.x - 2 * corner_dim) / edge_dim, 1});
	sprites.at(3).setScale(1, (current_dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(4).setScale((current_dimensions.x - 2 * corner_dim) / edge_dim, (current_dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(5).setScale(1, (current_dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(7).setScale((current_dimensions.x - 2 * corner_dim) / edge_dim, 1);

	// set position for the 9-slice console box
	sprites.at(0).setPosition(position.x, position.y);
	sprites.at(1).setPosition(position.x + corner_dim, position.y);
	sprites.at(2).setPosition(position.x + current_dimensions.x - corner_dim, position.y);

	sprites.at(3).setPosition(position.x, position.y + corner_dim);
	sprites.at(4).setPosition(position.x + corner_dim, position.y + corner_dim);
	sprites.at(5).setPosition(position.x + current_dimensions.x - corner_dim, position.y + corner_dim);

	sprites.at(6).setPosition(position.x, position.y + current_dimensions.y - corner_dim);
	sprites.at(7).setPosition(position.x + corner_dim, position.y + current_dimensions.y - corner_dim);
	sprites.at(8).setPosition(position.x + current_dimensions.x - corner_dim, position.y + current_dimensions.y - corner_dim);
}

} // namespace gui
