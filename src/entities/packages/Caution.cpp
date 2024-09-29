#include "Caution.hpp"
#include "Caution.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"
#include <algorithm>

namespace entity {
void Caution::update() {}

void Caution::avoid_player(player::Player& player) {}

void Caution::avoid_ledges(world::Map& map, shape::Collider& collider, dir::Direction& direction, int height) {
	heights.danger = height;
	testers.left.clear();
	testers.right.clear();
	heights.perceived = {};
	retreat = {};
	if (height < 1) { return; }
	auto step = sf::Vector2<float>{0.f, 32.f};
	auto buffer = sf::Vector2<float>{0.f, 8.f};
	for (int i{0}; i < height + 1; ++i) {
		testers.left.push_back({collider.vicinity.vertices.at(3) - buffer + step * static_cast<float>(i), false});
		testers.right.push_back({collider.vicinity.vertices.at(2) - buffer + step * static_cast<float>(i), false});
	}
	// only test cells later in the grid to save time
	auto start_index = (testers.left.at(0).first.x / 32.f) * (testers.right.at(0).first.y / 32.f) - 1;
	if (start_index < 0) { return; }
	auto& probe = direction.lr == dir::LR::left ? testers.left : testers.right;
	for (auto i{start_index}; i < map.get_layers().at(world::MIDDLEGROUND).grid.cells.size() - 1; ++i) {
		auto& cell = map.get_layers().at(world::MIDDLEGROUND).grid.cells.at(static_cast<int>(i));
		for (auto& test : probe) {
			if (cell.bounding_box.contains_point(test.first)) { test.second = true; }
		}
	}
	for (auto& test : probe) { 
		if (!test.second) {
			++heights.perceived;
		} else {
			break;
		}
	}
	if (heights.perceived > height) { retreat = {10.f, 0.f}; }
	if (direction.lr == dir::LR::right) { retreat.x *= -1.f; }
}

bool entity::Caution::danger() const { return heights.perceived > heights.danger; }

} // namespace entity