
#include <fornani/gui/Slider.hpp>
#include <algorithm>

namespace fornani::gui {

Slider::Slider(sf::Vector2f const position, float const size) {
	m_border.setOrigin({size, 0.f});
	m_current.setOrigin({size, 0.f});
	m_total.setOrigin({size, 0.f});

	m_border.setPosition(position);
	m_border.setFillColor(sf::Color::Transparent);
	m_border.setOutlineThickness(2.f);
	m_current.setPosition(position);
	m_total.setPosition(position);
}

void Slider::handle_input(input::InputSystem& input) {}

void Slider::update(float const percentage) {
	auto const normalized = std::clamp(percentage, 0.f, 1.f);

	m_current.setSize({m_total.getSize().x * normalized, m_total.getSize().y});

	auto const puck_radius = m_puck.getSize().x * 0.5f;
	m_puck.setPosition({m_total.getPosition().x + m_total.getSize().x * normalized - puck_radius, m_total.getPosition().y + (m_total.getSize().y - m_puck.getSize().y) * 0.5f});
}

void Slider::render(sf::RenderTarget& win) {
	win.draw(m_border);
	win.draw(m_total);
	win.draw(m_current);
	win.draw(m_puck);
}

} // namespace fornani::gui
