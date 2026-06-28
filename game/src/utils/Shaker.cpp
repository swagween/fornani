
#include <fornani/utils/Random.hpp>
#include <fornani/utils/Shaker.hpp>

namespace fornani {

Shaker::Shaker(ShakeParameters params) : m_params{params}, m_phase{params.frequency}, m_lifetime{params.lifetime}, m_energy{params.energy} {}

void Shaker::tick(sf::Vector2f& point) {
	if (m_phase.is_almost_complete()) {
		m_offset = random::random_vector_float(-m_energy, m_energy);
		m_phase.start();
		m_energy *= m_lifetime.get_normalized();
	}
	if (m_lifetime.is_complete()) { point = m_point; }
	point = m_point + m_offset;
	m_lifetime.update();
	m_phase.update();
}

void Shaker::tick_static(sf::Vector2f& point) {
	if (m_phase.is_almost_complete()) {
		m_offset = random::random_vector_float(-m_energy, m_energy);
		point = m_point + m_offset;
		m_phase.start();
		m_energy *= m_lifetime.get_normalized();
	}
	if (m_lifetime.is_complete()) { point = m_point; }
	m_lifetime.update();
	m_phase.update();
}

void Shaker::shake() {
	m_phase.start();
	m_lifetime.start();
}

} // namespace fornani
