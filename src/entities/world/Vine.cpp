#include "Vine.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"

namespace entity {

Vine::Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length) : length(length), position(position), chain({0.9f, 0.02f, 24.f}, length) {
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize({8.f, 24.f});
}

void Vine::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) { chain.update(svc, map, player); }

void Vine::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	chain.render(svc, win, cam);
	for (int i{0}; i < length; ++i) {
		drawbox.setPosition(position + spacing * static_cast<float>(i) - cam);
		win.draw(drawbox);
	}
}

} // namespace entity
