
#include "fornani/graphics/CameraController.hpp"

namespace fornani::graphics {

void CameraController::shake(ShakeProperties properties) { shake_properties = properties; }

void CameraController::shake(int frequency, float energy, int start_time, int dampen_factor) { shake_properties = {true, frequency, energy, start_time, dampen_factor}; }

void CameraController::cancel() { shake_properties = {}; }

void CameraController::free() { m_state = CameraState::free; }

void CameraController::constrain() { m_state = CameraState::constrained; }

void CameraController::set_owner(CameraOwner to) { m_owner = to; }

void CameraController::add_anchor(sf::Vector2f position, float weight) {
	m_anchor_position += position * weight;
	m_anchor_weight += weight;
}

std::optional<sf::Vector2f> CameraController::get_anchor_position() const {
	if (m_anchor_weight <= 0.f) { return std::nullopt; }
	return m_anchor_position / m_anchor_weight;
}

void CameraController::clear_anchors() {
	m_anchor_position = {};
	m_anchor_weight = 0.f;
}

} // namespace fornani::graphics
