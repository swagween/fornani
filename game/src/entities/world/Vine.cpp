#include "fornani/entities/world/Vine.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/utils/Math.hpp"

namespace entity {

Vine::Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length, int size, bool foreground, bool reversed)
	: length(length), size(size), position(position), chain(svc, {0.995f, 0.08f, static_cast<float>(size) * 0.5f, 14.f}, position, length, reversed), sprite{size == 1 ? svc.assets.t_vine : svc.assets.t_vine_large}
{
	drawbox.setOutlineColor(svc.styles.colors.blue);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setSize({8.f, 8.f});
	if (reversed) { flags.set(VineFlags::reverse); }
	auto index = util::Circuit(4);
	auto last_index = svc.random.random_range(0, 3);
	auto ctr{0};
	for (auto& link : chain.links) {
		index.set(svc.random.random_range(0, index.get_order()));
		if (index.get() == last_index) { index.modulate(1); }
		auto sign = svc.random.percent_chance(50) ? -1 : 1;
		encodings.push_back({index.get(), sign});
		last_index = index.get();
		// optionally add treasure container to vine segment
		if (svc.random.percent_chance(5)) {
			auto rarity = item::Rarity::common;
			auto random_sample = svc.random.random_range_float(0.0f, 1.0f);
			if (random_sample < constants.priceless) {
				rarity = item::Rarity::priceless;
			} else if (random_sample < constants.rare) {
				rarity = item::Rarity::rare;
			} else if (random_sample < constants.uncommon) {
				rarity = item::Rarity::uncommon;
			} else {
				rarity = item::Rarity::common;
			}
			if (!treasure_balls) { treasure_balls = std::vector<std::unique_ptr<TreasureContainer>>{}; }
			treasure_balls.value().push_back(std::make_unique<TreasureContainer>(svc, rarity, link.get_anchor(), ctr));
		}
		++ctr;
	}
	sprite.setOrigin({16.f * static_cast<float>(size), 16.f * static_cast<float>(size)});
	if (foreground) { flags.set(VineFlags::foreground); }
}

void Vine::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	chain.update(svc, map, player);
	if (treasure_balls) {
		for (auto& ball : treasure_balls.value()) { ball->update(svc, chain.links.at(ball->get_index()).get_bob()); }
		std::erase_if(treasure_balls.value(), [](auto const& b) { return b->destroyed(); });
	}
	if(spawnable_platforms) {
		for (auto& plat : spawnable_platforms.value()) { plat->update(svc, player, chain.links.at(plat->get_index()).get_bob()); }
	}
}

void Vine::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (treasure_balls) {
		for (auto& ball : treasure_balls.value()) { ball->on_hit(svc, map, proj); }
	}
	if (spawnable_platforms) {
		for (auto& plat : spawnable_platforms.value()) { plat->on_hit(svc, map, proj); }
	}
}

void Vine::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		chain.render(svc, win, cam);
	}
	if (treasure_balls) {
		for (auto& ball : treasure_balls.value()) { ball->render(svc, win, cam); }
	}
	int ctr{0};
	if (!svc.greyblock_mode()) {
		for (auto& link : chain.links) {
			sprite.setTextureRect(sf::IntRect({0, encodings.at(ctr).at(0) * 32 * size}, {32 * size, 32 * size}));
			sprite.setScale({static_cast<float>(encodings.at(ctr).at(1)), 1.f});
			sprite.setPosition(util::round_to_even(link.get_bob()) - cam);
			win.draw(sprite);
			++ctr;
		}
	}
	if (spawnable_platforms) {
		for (auto& plat : spawnable_platforms.value()) { plat->render(svc, win, cam); }
	}
}

void Vine::add_platform(automa::ServiceProvider& svc, int link_index) { 
	treasure_balls = {}; // don't want them to get in the way
	if (link_index > chain.links.size() || link_index < 0) { return; }
	auto& link = chain.links.at(link_index);
	if (!spawnable_platforms) { spawnable_platforms = std::vector<std::unique_ptr<SpawnablePlatform>>{}; }
	spawnable_platforms.value().push_back(std::make_unique<SpawnablePlatform>(svc, link.get_anchor(), link_index));
}

} // namespace entity
