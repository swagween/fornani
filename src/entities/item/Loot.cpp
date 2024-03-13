
#include "Loot.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace item {
Loot::Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos) {

	auto drop_rate = svc::randomLocator.get().random_range(5, 10);
	position = pos;

	std::string_view key{};
	for (int i = 0; i < drop_rate; ++i) {
		if (svc::randomLocator.get().percent_chance(6)) {
			key = "heart";
		} else {
			key = "orb";
		}
		float randx = svc::randomLocator.get().random_range_float(-100.0f, 100.0f);
		float randy = svc::randomLocator.get().random_range_float(-100.0f, 100.0f);
		drops.push_back(Drop(svc, key, probability));
		drops.back().set_position(pos);
		drops.back().get_collider().physics.apply_force({randx, randy});
	}
}

void Loot::update(world::Map& map, player::Player& player) {
	for (auto& drop : drops) {
		drop.update(map);
		if (drop.get_collider().bounding_box.overlaps(player.collider.bounding_box)) {
			player.give_drop(drop.get_type(), drop.get_value());
			drop.destroy();
		}
	}
	std::erase_if(drops, [](auto& d) { return d.expired(); });
}

void Loot::render(sf::RenderWindow& win, sf::Vector2<float> campos) {
	for (auto& drop : drops) { drop.render(win, campos); }
}

void Loot::set_position(sf::Vector2<float> pos) { position = pos; }

} // namespace item
