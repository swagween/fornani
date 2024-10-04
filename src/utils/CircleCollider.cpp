#include "CircleCollider.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include <algorithm>


namespace shape {

CircleCollider::CircleCollider(float radius) : sensor{radius} {}

void CircleCollider::update(automa::ServiceProvider& svc) {
	physics.update(svc);
	sensor.set_position(physics.position);
	boundary.first = physics.position - bound;
	boundary.second = physics.position + bound;
}

void CircleCollider::handle_map_collision(world::Map& map) {
	auto& grid = map.get_layers().at(world::MIDDLEGROUND).grid;
	auto range = get_collision_range(map);
	for (auto i{range.first}; i < range.second; ++i) {
		auto& cell = grid.get_cell(static_cast<int>(i));
		if (!cell.is_collidable()) { continue; }
		if (cell.is_ramp()) { continue; }
		cell.collision_check = true;
		if (cell.value > 0) { handle_collision(cell.bounding_box); }
	}
}

void CircleCollider::handle_collision(shape::Shape& box) {
	if (sensor.within_bounds(box)) {
		sensor.bounds.getPosition().y < box.top() || sensor.bounds.getPosition().y < box.bottom() ? physics.velocity.y *= -1.f * physics.elasticity : physics.velocity.x *= -1.f * physics.elasticity;
		flags.set(CircleColliderFlags::collided);
	}
}

void CircleCollider::render(sf::RenderWindow& win, sf::Vector2<float> cam) { sensor.render(win, cam); }

std::pair<size_t, size_t> CircleCollider::get_collision_range(world::Map& map) {
	auto& grid = map.get_layers().at(world::MIDDLEGROUND).grid;
	auto start_index = sf::Vector2<size_t>(static_cast<size_t>((boundary.first.x / 32)), static_cast<size_t>((boundary.first.y / 32)));
	auto end_index = sf::Vector2<size_t>(static_cast<size_t>((boundary.second.x / 32)), static_cast<size_t>((boundary.second.y / 32)));
	auto start = static_cast<size_t>(map.dimensions.x) * start_index.y + start_index.x;
	auto end = static_cast<size_t>(map.dimensions.x) * end_index.y + end_index.x + 1;
	start = std::clamp(start, std::size_t{0}, static_cast<size_t>(grid.cells.size() - 1));
	end = std::clamp(end, std::size_t{0}, static_cast<size_t>(grid.cells.size() - 1));
	return {start, end};
}

} // namespace shape
