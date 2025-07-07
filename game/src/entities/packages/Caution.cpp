
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::entity {

void Caution::update() {}

void Caution::avoid_player(player::Player& player) {}

void Caution::avoid_ledges(world::Map& map, shape::Collider& collider, Direction const& direction, int height) {
	heights.danger = height;
	heights.perceived = {};
	retreat = {};
	if (height < 1 || map.dimensions.x < 1 || map.dimensions.y < 1) { return; }
	auto buffer = sf::Vector2f{0.f, 8.f};
	testers.left = collider.vicinity.vertices.at(3) - buffer;
	testers.right = collider.vicinity.vertices.at(2) - buffer;

	// only test cells later in the grid to save time
	auto& probe = direction.left() ? testers.left : testers.right;
	auto start_index = map.get_index_at_position(probe);
	for (auto i{start_index}; i < map.get_middleground()->grid.cells.size(); i += map.dimensions.x) {
		auto& cell = map.get_middleground()->grid.cells.at(static_cast<int>(i));
		if (cell.is_solid() || cell.is_platform()) {
			break;
		} else {
			++heights.perceived;
		}
	}
	if (heights.perceived >= height) { retreat = {10.f, 0.f}; }
	if (direction.right()) { retreat.x *= -1.f; }
}

Direction Caution::projectile_detected(world::Map& map, shape::Shape& zone, arms::Team friendly_fire) {
	auto ret = Direction{};
	for (auto& proj : map.active_projectiles) {
		if (proj.get_bounding_box().overlaps(zone) && proj.get_team() != friendly_fire) { ret = proj.get_direction(); }
	}
	return ret;
}

bool Caution::detected_step(world::Map& map, shape::Collider& collider, Direction const& direction, sf::Vector2f offset, int vision) {
	auto buffer = sf::Vector2f{collider.dimensions.x, 0.f};
	testers.left = collider.get_center() - (buffer + sf::Vector2f{offset.x, -offset.y});
	testers.right = collider.get_center() + buffer + offset;

	// only test cells later in the grid to save time
	auto& probe = direction.left() ? testers.left : testers.right;
	auto start_index = map.get_index_at_position(probe);
	auto line_of_sight = direction.left() ? -1 : 1;
	for (auto i{start_index}; line_of_sight == 1 ? i < start_index + vision : i > start_index - vision; i += line_of_sight) {
		auto& cell = map.get_middleground()->grid.cells.at(static_cast<int>(i));
		if (cell.is_solid() || cell.is_platform()) { return true; }
	}
	return false;
}

bool Caution::detected_ceiling(world::Map& map, shape::Collider& collider, sf::Vector2f offset, int vision) {
	testers.top = collider.get_center() - offset;
	auto start_index = map.get_index_at_position(testers.top);
	for (auto i{start_index}; i >= start_index - (map.dimensions.x * vision); i -= map.dimensions.x) {
		auto& cell = map.get_middleground()->grid.cells.at(static_cast<int>(i));
		if (cell.is_solid() || cell.is_platform()) { return true; }
	}
	return false;
}

void Caution::debug_render(sf::RenderWindow& win, sf::Vector2f cam) {
	sf::CircleShape probe{};
	probe.setFillColor(colors::mythic_green);
	probe.setRadius(4.f);
	probe.setOrigin({2.f, 2.f});
	probe.setPosition(testers.left - cam);
	win.draw(probe);
	probe.setPosition(testers.right - cam);
	win.draw(probe);
	probe.setPosition(testers.top - cam);
	win.draw(probe);
}

bool entity::Caution::danger() const { return heights.perceived >= heights.danger; }

} // namespace fornani::entity
