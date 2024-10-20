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

void CircleCollider::handle_map_collision(world::Map& map) { map.handle_cell_collision(*this); }

void CircleCollider::handle_collision(shape::Shape& shape) {
	if (sensor.within_bounds(shape)) {
		if (shape.non_square()) {
			physics.velocity.y *= -1.f * physics.elasticity;
			flags.set(CircleColliderFlags::collided);
			return;
		}
		sensor.bounds.getPosition().y < shape.top() || sensor.bounds.getPosition().y < shape.bottom() ? physics.velocity.y *= -1.f * physics.elasticity : physics.velocity.x *= -1.f * physics.elasticity;
		flags.set(CircleColliderFlags::collided);
	}
}

void CircleCollider::render(sf::RenderWindow& win, sf::Vector2<float> cam) { sensor.render(win, cam); }

} // namespace shape
