#include "fornani/utils/CollisionDepth.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Shape.hpp"

namespace fornani::util {

constexpr auto crush_threshold_v = sf::Vector2f{8.f, 16.f};

void CollisionDepth::calculate(shape::Collider const& native, shape::Shape const& other) {
	if (iterations.get_count() == 0) { collision_direction = CollisionDirection::none; }
	if (other.top() < native.hurtbox.bottom() && other.bottom() > native.hurtbox.top() && other.get_center().x < native.get_center().x) { candidate.left = other.right() - native.left(); }
	if (other.top() < native.hurtbox.bottom() && other.bottom() > native.hurtbox.top() && other.get_center().x > native.get_center().x) { candidate.right = other.left() - native.right(); }
	if (other.left() < native.hurtbox.right() && other.right() > native.hurtbox.left() && other.get_center().y < native.get_center().y) { candidate.top = other.bottom() - native.top(); }
	if (other.left() < native.hurtbox.right() && other.right() > native.hurtbox.left() && other.get_center().y > native.get_center().y) { candidate.bottom = other.top() - native.bottom(); }
	try_push();
	iterations.update();
	collision_direction = std::abs(out_depth.left) + std::abs(out_depth.right) > std::abs(out_depth.top) + std::abs(out_depth.bottom) ? CollisionDirection::vertical : CollisionDirection::horizontal;
}

void CollisionDepth::update() {
	if (stream.empty()) { return; }
	out_depth = candidate;
	for (auto& depth : stream) {
		out_depth.left = std::max(depth.left, out_depth.left);
		out_depth.right = std::min(depth.right, out_depth.right);
		out_depth.top = std::max(depth.top, out_depth.top);
		out_depth.bottom = std::min(depth.bottom, out_depth.bottom);
	}
	stream.clear();
}

void CollisionDepth::maximize(CollisionDepth& other) {
	out_depth.left = std::max(other.out_depth.left, out_depth.left);
	out_depth.right = std::min(other.out_depth.right, out_depth.right);
	out_depth.top = std::max(other.out_depth.top, out_depth.top);
	out_depth.bottom = std::min(other.out_depth.bottom, out_depth.bottom);
}

void CollisionDepth::print() {
	NANI_LOG_INFO(m_logger, "Stream size: {}", stream.size());
	NANI_LOG_INFO(m_logger, "-Out Depth-");
	NANI_LOG_INFO(m_logger, "Left..: {}", out_depth.left);
	NANI_LOG_INFO(m_logger, "Right.: {}", out_depth.right);
	NANI_LOG_INFO(m_logger, "Top...: {}", out_depth.top);
	NANI_LOG_INFO(m_logger, "Bottom: {}", out_depth.bottom);
	return;
}

static auto is_within_crush_range(float const test, bool positive) { return positive ? test > crush_threshold_v.x && test < crush_threshold_v.y : test < -crush_threshold_v.x && test > -crush_threshold_v.y; }

bool CollisionDepth::crushed() const {
	return (is_within_crush_range(out_depth.bottom, false) && is_within_crush_range(out_depth.top, true)) || (is_within_crush_range(out_depth.right, false) && is_within_crush_range(out_depth.left, true));
}

void CollisionDepth::render(shape::Shape const& bounding_box, sf::RenderWindow& win, sf::Vector2f cam) {
	collision_ray.setFillColor(sf::Color::Cyan);

	// left
	collision_ray.setSize({out_depth.left, 2.f});
	collision_ray.setPosition({bounding_box.left() - cam.x, bounding_box.top() + bounding_box.get_dimensions().y * 0.5f - cam.y});
	win.draw(collision_ray);
	// right
	collision_ray.setSize({out_depth.right, 2.f});
	collision_ray.setPosition({bounding_box.right() - cam.x, bounding_box.top() + bounding_box.get_dimensions().y * 0.5f - cam.y});
	win.draw(collision_ray);
	// top
	collision_ray.setSize({2.f, out_depth.top});
	collision_ray.setPosition({bounding_box.left() - cam.x + bounding_box.get_dimensions().x * 0.5f, bounding_box.top() - cam.y});
	win.draw(collision_ray);
	// bottom
	collision_ray.setSize({2.f, out_depth.bottom});
	collision_ray.setPosition({bounding_box.left() - cam.x + bounding_box.get_dimensions().x * 0.5f, bounding_box.bottom() - cam.y});
	win.draw(collision_ray);
}

bool CollisionDepth::horizontal_squish() const { return collision_direction == CollisionDirection::horizontal; }

bool CollisionDepth::vertical_squish() const { return collision_direction == CollisionDirection::vertical; }

void CollisionDepth::try_push() { stream.push_back(candidate); }

} // namespace fornani::util
