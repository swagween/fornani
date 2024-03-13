
#include "Loot.hpp"
#include "../../setup/ServiceLocator.hpp"
#include "../../service/ServiceProvider.hpp"

namespace item {
Loot::Loot(automa::ServiceProvider& svc, sf::Vector2<int> drop_range, float probability, sf::Vector2<float> pos) {

	auto drop_rate = svc::randomLocator.get().random_range(5, 10);
	position = pos;

	std::string_view key{};
	if (svc::randomLocator.get().percent_chance(6)) {
		key = "heart";
	} else {
		key = "orb";
	}
	for (int i = 0; i < drop_rate; ++i) {
		drops.push_back(Drop(svc, key, probability));
		drops.back().set_position(pos);
	}
}

void Loot::update() {
	for (auto& drop : drops) { drop.update(); }
}

void Loot::render(sf::RenderWindow& win, sf::Vector2<float> campos) {
	for (auto& drop : drops) { drop.render(win, campos); }
}

void Loot::set_position(sf::Vector2<float> pos) { position = pos; }

std::vector<Drop>& Loot::get_drops() { return drops; }
} // namespace item
