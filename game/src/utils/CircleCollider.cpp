
#include "fornani/utils/CircleCollider.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::shape {

CircleCollider::CircleCollider(float radius) : sensor{radius} { sensor.bounds.setOrigin({radius, radius}); }

void CircleCollider::update(automa::ServiceProvider& svc) {
	physics.update(svc);
	sensor.set_position(physics.position);
	boundary.first = physics.position - bound;
	boundary.second = physics.position + bound;
}

void CircleCollider::handle_map_collision(world::Map& map) {
	flags.reset(CircleColliderFlags::collided);
	map.handle_cell_collision(*this);
}

void CircleCollider::handle_collision(shape::Shape& shape, bool soft) {
	if (!sensor.within_bounds(shape)) { return; }
	auto distance = util::magnitude(sensor.bounds.getPosition() - shape.get_center());
	auto circle_left_of = sensor.bounds.getPosition().x < shape.get_center().x;
	auto circle_right_of = !circle_left_of;
	auto circle_above = sensor.bounds.getPosition().y < shape.get_center().y;
	auto circle_below = !circle_above;
	auto mtv = sensor.get_MTV(shape);
	auto leeway = soft ? 0.f : 1.5f;
	auto nudge = soft ? 1.f : 0.f;
	auto vertical = abs(mtv.y) > abs(mtv.x);
	if (shape.non_square()) {
		vertical ? physics.position.y -= mtv.y* leeway + nudge : physics.position.x -= mtv.x * leeway + nudge;
	} else {
		physics.position.x += circle_right_of ? abs(mtv.x) * leeway + nudge : abs(mtv.x) * -leeway - nudge;
		physics.position.y += circle_below ? abs(mtv.y) * leeway + nudge : abs(mtv.y) * -leeway - nudge;
	}
	if (!soft) { vertical ? physics.collide({0, 1}) : physics.collide({1, 0}); }
	flags.set(CircleColliderFlags::collided);
	sensor.set_position(physics.position);
}

void CircleCollider::render(sf::RenderWindow& win, sf::Vector2f cam) { sensor.render(win, cam); }

auto CircleCollider::get_collision_result(Shape& shape) const -> sf::Vector2i {
	if (!sensor.within_bounds(shape)) { return {}; }
	auto ret = sf::Vector2i{};
	ret.x = get_global_center().x < shape.get_center().x ? -1 : 1;
	ret.y = get_global_center().y < shape.get_center().y ? -1 : 1;
	auto side_collision = std::abs(get_global_center().x - shape.get_center().x) > std::abs(get_global_center().y - shape.get_center().y);
	if (side_collision) {
		ret.y = 0;
	} else {
		ret.x = 0;
	}
	return ret;
}

[[nodiscard]] auto CircleCollider::get_global_center() const -> sf::Vector2f { return physics.position; }

[[nodiscard]] auto CircleCollider::get_local_center() const -> sf::Vector2f { return sensor.get_local_center(); }

} // namespace fornani::shape
