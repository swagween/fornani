
#include "Console.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../service/ServiceProvider.hpp"

namespace gui {

Console::Console() {
	for (auto& sprite : sprites) { sprite.setTexture(svc::assetLocator.get().t_ui); }
	sprites.at(0).setTextureRect(sf::IntRect{{0, 0}, {corner_factor, corner_factor}});
	sprites.at(1).setTextureRect(sf::IntRect{{corner_factor, 0}, {edge_factor, corner_factor}});
	sprites.at(2).setTextureRect(sf::IntRect{{corner_factor + edge_factor, 0}, {corner_factor, corner_factor}});
	sprites.at(3).setTextureRect(sf::IntRect{{0, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(4).setTextureRect(sf::IntRect{{corner_factor, corner_factor}, {edge_factor, edge_factor}});
	sprites.at(5).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor}, {corner_factor, edge_factor}});
	sprites.at(6).setTextureRect(sf::IntRect{{0, corner_factor + edge_factor}, {corner_factor, corner_factor}});
	sprites.at(7).setTextureRect(sf::IntRect{{corner_factor, corner_factor + edge_factor}, {edge_factor, corner_factor}});
	sprites.at(8).setTextureRect(sf::IntRect{{corner_factor + edge_factor, corner_factor + edge_factor}, {corner_factor, corner_factor}});

	dimensions = sf::Vector2<float>{(float)cam::screen_dimensions.x - 2 * pad, (float)cam::screen_dimensions.y / height_factor};
	position = sf::Vector2<float>{origin.x, origin.y - dimensions.y};
	text_origin = sf::Vector2<float>{20.0f, 20.0f};

	extent = corner_factor * 2;
}

void Console::begin() {
	dimensions.y = corner_factor * 2;
	flags.set(ConsoleFlags::active);
	writer.start();
}

void Console::update() {
	writer.set_bounds(position + sf::Vector2<float>{dimensions.x - 2 * border.left, dimensions.y - 2 * border.top});
	writer.set_position(position + sf::Vector2<float>{border.left, border.top});
	if (flags.test(ConsoleFlags::active)) { extent += speed; }
	if (extent < (float)cam::screen_dimensions.y / height_factor) {
		dimensions.y = extent;
	} else {
		dimensions.y = (float)cam::screen_dimensions.y / height_factor;
	}
	nine_slice(corner_factor, edge_factor);
	writer.selection_mode() ? flags.set(ConsoleFlags::selection_mode) : flags.reset(ConsoleFlags::selection_mode);
	writer.update();
}

void Console::render(sf::RenderWindow& win) {
	for (auto& sprite : sprites) { win.draw(sprite); }
}

void Console::load_and_launch(automa::ServiceProvider& svc, std::string_view key) {
	if (!flags.test(ConsoleFlags::loaded)) {
		writer.load_message(svc, svc.text.console, key);
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
	extent = dimensions.y = corner_factor * 2;
	flags.reset(ConsoleFlags::active);
	flags.reset(ConsoleFlags::loaded);
}

void Console::nine_slice(int corner_dim, int edge_dim) {

	// set sizes for stretched 9-slice sprites
	sprites.at(1).setScale({(dimensions.x - 2 * corner_dim) / edge_dim, 1});
	sprites.at(3).setScale(1, (dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(4).setScale((dimensions.x - 2 * corner_dim) / edge_dim, (dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(5).setScale(1, (dimensions.y - 2 * corner_dim) / edge_dim);
	sprites.at(7).setScale((dimensions.x - 2 * corner_dim) / edge_dim, 1);

	// set position for the 9-slice console box
	sprites.at(0).setPosition(position.x, position.y);
	sprites.at(1).setPosition(position.x + corner_dim, position.y);
	sprites.at(2).setPosition(position.x + dimensions.x - corner_dim, position.y);

	sprites.at(3).setPosition(position.x, position.y + corner_dim);
	sprites.at(4).setPosition(position.x + corner_dim, position.y + corner_dim);
	sprites.at(5).setPosition(position.x + dimensions.x - corner_dim, position.y + corner_dim);

	sprites.at(6).setPosition(position.x, position.y + dimensions.y - corner_dim);
	sprites.at(7).setPosition(position.x + corner_dim, position.y + dimensions.y - corner_dim);
	sprites.at(8).setPosition(position.x + dimensions.x - corner_dim, position.y + dimensions.y - corner_dim);
}

} // namespace gui
