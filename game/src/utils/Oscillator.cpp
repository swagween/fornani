
#include <fornani/utils/Oscillator.hpp>

namespace fornani {

void Oscillator::update(float dt) {
	float delta = m_target - m_position;

	// shortest cyclic path
	if (delta > 0.5f) delta -= 1.f;
	if (delta < -0.5f) delta += 1.f;

	float accel = m_stiffness * delta - m_damping * m_velocity;

	m_velocity += accel * dt;
	m_position += m_velocity * dt;

	// wrap position into 0–1
	if (m_position >= 1.f) m_position -= 1.f;
	if (m_position < 0.f) m_position += 1.f;
}

} // namespace fornani
