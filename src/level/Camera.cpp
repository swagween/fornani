#include "Camera.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace fornani {

Camera::Camera() {
	gravitator = vfx::Gravitator(gravitator.collider.physics.position, sf::Color::Transparent, CAM_GRAV);
	gravitator.collider.physics = components::PhysicsComponent({CAM_FRICTION, CAM_FRICTION}, CAM_MASS);
}

void Camera::update(automa::ServiceProvider& svc) {
	screen_dimensions = svc.constants.screen_dimensions;
	auto screen_dimensions_f = sf::Vector2<float>(screen_dimensions);
	bounding_box.setSize(screen_dimensions_f);
	gravitator.update(svc);
	bounding_box.setPosition(gravitator.collider.physics.position);
	observed_velocity = bounding_box.getPosition() - previous_position;
	shake.timer.update();
	previous_position = bounding_box.getPosition();
	if (shake.timer.running()) {
		flags.set(CamFlags::shake);
	} else {
		flags.reset(CamFlags::shake);
		display_position = bounding_box.getPosition() + map_bounds_offset;
	}
	if (shake.timer.running() && shake.timer.get_cooldown() % shake.frequency == 0) {
		shake.dampen.update();
		auto diff = (float)shake.dampen.get_cooldown() * shake.energy;
		if (abs(diff) < 0.1f) {
			shake.timer.cancel();
			diff = 0.f;
		}
		auto randx = svc.random.random_range_float(-diff, diff);
		auto randy = svc.random.random_range_float(-diff, diff);
		display_position = bounding_box.getPosition() + sf::Vector2<float>{randx, randy} + map_bounds_offset;
	}
}

void Camera::restrict_movement(sf::Vector2<float>& bounds) {
	auto top_left = bounding_box.getPosition();
	auto bottom_right = bounds - bounding_box.getSize();
	bottom_right = {std::max(bottom_right.x, 0.f), std::max(bottom_right.y, 0.f)};
	sf::Vector2<float> clamped_pos = {std::clamp(top_left.x, 0.f, bottom_right.x), std::clamp(top_left.y, 0.f, bottom_right.y)};
	bounding_box.setPosition(clamped_pos);
	gravitator.set_position(clamped_pos);
	target = clamped_pos;
	if (bounds.x < bounding_box.getSize().x) { fix_horizontally(bounds); }
	if (bounds.y < bounding_box.getSize().y) { fix_vertically(bounds); }
	if (!flags.test(CamFlags::shake)) { display_position = bounding_box.getPosition() + map_bounds_offset; }
}

void Camera::fix_horizontally(sf::Vector2<float> map_dim) { map_bounds_offset.x = (map_dim.x - bounding_box.getSize().x) * 0.5f; }

void Camera::fix_vertically(sf::Vector2<float> map_dim) { map_bounds_offset.y = (map_dim.y - bounding_box.getSize().y) * 0.5f; }

void Camera::set_position(sf::Vector2<float> new_pos) {
	gravitator.set_position(new_pos);
	bounding_box.setPosition(new_pos);
	display_position = new_pos;
}

void Camera::center(sf::Vector2<float> new_position) {
	auto aim = new_position - bounding_box.getSize() * 0.5f;
	gravitator.set_target_position(aim);
}

void Camera::force_center(sf::Vector2<float> new_position) { set_position(new_position - bounding_box.getSize() * 0.5f); }

void Camera::begin_shake() {
	shake.timer.start(shake.start_time);
	shake.dampen.start(shake.dampen_factor);
}

} // namespace fornani
