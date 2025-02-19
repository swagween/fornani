#include "fornani/entities/world/Grass.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/world/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::entity {

Grass::Grass(automa::ServiceProvider& svc, sf::Vector2<float> position, int length, int size, bool foreground)
	: position(position), length(length), size(size), chain(svc, {0.98f, 0.84f, static_cast<float>(size) * 0.001f, -3.2f}, position + sf::Vector2<float>{16.f, 32.f}, length, true),
	  sprite{size == 1 ? svc.assets.t_grass : svc.assets.t_grass_large} {
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize({8.f, 8.f});
	auto const index = util::Random::random_range(0, 3);
	auto const sign = util::Random::percent_chance(50) ? -1 : 1;
	for (auto& link : chain.links) { encodings.push_back({index, sign}); }
	sprite.setOrigin({16.f * static_cast<float>(size), constants.f_height * 0.5f * static_cast<float>(size)});
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
		sprite.setTextureRect(sf::IntRect({static_cast<int>(type) * 32, encodings.at(ctr).at(0) * 32 * size + 32 - ctr * constants.height}, {32 * size, constants.height * size}));
		sprite.setScale({static_cast<float>(encodings.at(ctr).at(1)), 1.f});
		constexpr auto tweak = sf::Vector2{0.f, 4.f};
		sprite.setPosition(util::round_to_even(link.get_bob()) - cam + tweak);
		win.draw(sprite);
		++sprite_ctr;
		++ctr;
	}
}

} // namespace fornani::entity
