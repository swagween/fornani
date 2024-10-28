#include "Grass.hpp"
#include "../../level/Map.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../utils/Math.hpp"

namespace entity {

Grass::Grass(automa::ServiceProvider& svc, sf::Vector2<float> position, int length, int size, bool foreground)
	: length(length), size(size), position(position), chain(svc, {0.98f, 0.84f, static_cast<float>(size) * 0.001f, -3.2f}, position + sf::Vector2<float>{16.f, 32.f}, length, true) {
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize({8.f, 8.f});
	for (auto& sprite : sprites) { size == 1 ? sprite.setTexture(svc.assets.t_grass) : sprite.setTexture(svc.assets.t_grass_large); }
	auto index = svc.random.random_range(0, 3);
	auto sign = svc.random.percent_chance(50) ? -1 : 1;
	for (auto& link : chain.links) { encodings.push_back({index, sign}); }
	for (auto& sprite : sprites) { sprite.setOrigin({16.f * static_cast<float>(size), constants.f_height * 0.5f * static_cast<float>(size)}); }
	if (foreground) { flags.set(GrassFlags::foreground); }
}

void Grass::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) { chain.update(svc, map, player, 0.5f); }

void Grass::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		chain.render(svc, win, cam);
		return;
	}
	int ctr{0};
	for (auto& link : chain.links) {
		auto sprite_ctr{0};
			sprites.at(ctr).setTextureRect(sf::IntRect({static_cast<int>(type) * 32, encodings.at(ctr).at(0) * 32 * size + 32 - ctr * constants.height}, {32 * size, constants.height * size}));
			sprites.at(ctr).setScale({static_cast<float>(encodings.at(ctr).at(1)), 1.f});
			auto tweak = sf::Vector2<float>{0.f, 4.f};
			sprites.at(ctr).setPosition(util::round_to_even(link.get_bob()) - cam + tweak);
			win.draw(sprites.at(ctr));
			++sprite_ctr;
		++ctr;
	}
}

} // namespace entity
