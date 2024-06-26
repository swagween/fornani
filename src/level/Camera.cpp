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
	previous_position = bounding_box.getPosition();
	gravitator.update(svc);
	bounding_box.setPosition(gravitator.collider.physics.position);
	observed_velocity = bounding_box.getPosition() - previous_position;
}

void Camera::restrict_movement(sf::Vector2<float>& bounds) {
	auto top_left = bounding_box.getPosition();
	auto bottom_right = bounds - bounding_box.getSize();
	bottom_right = {std::max(bottom_right.x, 0.f), std::max(bottom_right.y, 0.f)};
	sf::Vector2<float> clamped_pos = {std::clamp(top_left.x, 0.f, bottom_right.x), std::clamp(top_left.y, 0.f, bottom_right.y)};
	bounding_box.setPosition(clamped_pos);
	gravitator.set_position(clamped_pos);
	if (bounds.x < bounding_box.getSize().x) { fix_horizontally(bounds); }
	if (bounds.y < bounding_box.getSize().y) { fix_vertically(bounds); }
}

void Camera::fix_horizontally(sf::Vector2<float> map_dim) { bounding_box.setPosition((map_dim.x - bounding_box.getSize().x) * 0.5f, bounding_box.getPosition().y); }

void Camera::fix_vertically(sf::Vector2<float> map_dim) { bounding_box.setPosition(bounding_box.getPosition().x, (map_dim.y - bounding_box.getSize().y) * 0.5f); }

void Camera::set_position(sf::Vector2<float> new_pos) {
	gravitator.set_position(new_pos);
	bounding_box.setPosition(new_pos);
}

void Camera::center(sf::Vector2<float> new_position) { gravitator.set_target_position(new_position - bounding_box.getSize() * 0.5f); }

void Camera::force_center(sf::Vector2<float> new_position) { set_position(new_position - bounding_box.getSize() * 0.5f); }

} // namespace fornani
