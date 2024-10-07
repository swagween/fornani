#include "Vine.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../utils/Math.hpp"

namespace entity {

Vine::Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length, int size, bool foreground) : length(length), size(size), position(position), chain({0.98f, 0.14f, static_cast<float>(size) * 12.f}, position, length) {
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize({8.f, 8.f});
	size == 1 ? sprite.setTexture(svc.assets.t_vine) : sprite.setTexture(svc.assets.t_vine_large);
	for (auto& link : chain.links) {
		auto index = svc.random.random_range(0, 3);
		auto sign = svc.random.percent_chance(50) ? -1 : 1;
		encodings.push_back({index, sign});
	}
	sprite.setOrigin({16.f * static_cast<float>(size), 16.f * static_cast<float>(size)});
	if (foreground) { flags.set(VineFlags::foreground); }
}

void Vine::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) { chain.update(svc, map, player); }

void Vine::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	//chain.render(svc, win, cam);
	int ctr{0};
	for (auto& link : chain.links) {
		sprite.setTextureRect(sf::IntRect({0, encodings.at(ctr).at(0) * 32 * size}, {32 * size, 32 * size}));
		sprite.setScale({static_cast<float>(encodings.at(ctr).at(1)), 1.f});
		sprite.setPosition(util::round_to_even(link.get_bob()) - cam);
		win.draw(sprite);
		++ctr;
	}
}

} // namespace entity
