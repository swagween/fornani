#include "fornani/world/Camera.hpp"

#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani {

Camera::Camera() { m_physics.set_global_friction(0.89f); }

void Camera::update(automa::ServiceProvider& svc) {
	m_view.size = svc.window->f_screen_dimensions();
	m_state = svc.camera_controller.get_state();
	if (svc.camera_controller.is_shaking()) {
		m_shake.properties = svc.camera_controller.get_shake_properties();
		begin_shake();
		svc.camera_controller.cancel();
	}
	m_shake.timer.update();
	if (m_shake.timer.running()) {
		m_flags.set(CameraFlags::shake);
	} else {
		m_flags.reset(CameraFlags::shake);
		m_final_position = m_physics.position;
	}
	if (m_shake.timer.running() && m_shake.timer.get_cooldown() % m_shake.properties.frequency == 0) {
		m_shake.dampen.update();
		auto diff = static_cast<float>(m_shake.dampen.get_cooldown()) * m_shake.properties.energy;
		if (ccm::abs(diff) < 0.1f) {
			m_shake.timer.cancel();
			diff = 0.f;
		}
		auto randx = util::random::random_range_float(-diff, diff);
		auto randy = util::random::random_range_float(-diff, diff);
		m_final_position = m_physics.position + sf::Vector2f{randx, randy};
	}
	m_physics.simple_update();
}

void Camera::set_bounds(sf::Vector2f to_bounds) { m_bounds.size = to_bounds; }

void Camera::center(sf::Vector2f new_position) {
	auto const aim = new_position - m_view.getCenter();
	m_forced_target_position = aim;
	m_target_position = get_clamped_position(aim);
	auto const to_position = m_state == graphics::CameraState::free ? m_forced_target_position : m_target_position;
	m_steering.target(m_physics, to_position, 0.002f);
}

void Camera::force_center(sf::Vector2f new_position) {
	auto aim = new_position - m_view.getCenter();
	m_forced_target_position = aim;
	m_target_position = get_clamped_position(aim);
	m_physics.position = m_target_position;
}

void Camera::begin_shake() {
	m_shake.timer.start(m_shake.properties.start_time);
	m_shake.dampen.start(m_shake.properties.dampen_factor);
}

auto Camera::get_clamped_position(sf::Vector2f const position) const -> sf::Vector2f {
	auto free_xpan = m_bounds.size.x > m_view.size.x;
	auto free_ypan = m_bounds.size.y > m_view.size.y;
	auto top_left = m_bounds.position;
	top_left.x = free_xpan ? m_bounds.position.x : (m_bounds.size.x - m_view.size.x) / 2.f;
	top_left.y = free_ypan ? m_bounds.position.x : (m_bounds.size.y - m_view.size.y) / 2.f;
	auto bottom_right = top_left + m_bounds.size - m_view.size;
	bottom_right.x = free_xpan ? bottom_right.x : top_left.x;
	bottom_right.y = free_ypan ? bottom_right.y : top_left.y;
	return sf::Vector2f{ccm::ext::clamp(position.x, top_left.x, bottom_right.x), ccm::ext::clamp(position.y, top_left.y, bottom_right.y)};
}

} // namespace fornani
