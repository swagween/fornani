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
	heights.perceived = {};
	retreat = {};
	if (height < 1 || map.dimensions.x < 1 || map.dimensions.y < 1) { return; }
	auto step = sf::Vector2<float>{0.f, 32.f};
	auto buffer = sf::Vector2<float>{0.f, 8.f};
	testers.left = collider.vicinity.vertices.at(3) - buffer;
	testers.right = collider.vicinity.vertices.at(2) - buffer;

	// only test cells later in the grid to save time
	auto& probe = direction.lr == dir::LR::left ? testers.left : testers.right;
	auto start_index = map.get_index_at_position(probe);
	for (auto i{start_index}; i < map.get_layers().at(world::MIDDLEGROUND).grid.cells.size(); i += map.dimensions.x) {
		auto& cell = map.get_layers().at(world::MIDDLEGROUND).grid.cells.at(static_cast<int>(i));
		if (cell.is_solid()) {
			break;
		} else {
			++heights.perceived;
		}
	}
	if (heights.perceived >= height) { retreat = {10.f, 0.f}; }
	if (direction.lr == dir::LR::right) { retreat.x *= -1.f; }
}

bool entity::Caution::danger() const { return heights.perceived >= heights.danger; }

} // namespace entity