
#include <fornani/audio/Soundboard.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/gui/Slider.hpp>
#include <fornani/systems/InputSystem.hpp>
#include <algorithm>

namespace fornani::gui {

constexpr auto slider_height_v = 6.f;

Slider::Slider(float const size, sf::Vector2f const position) : m_size{size} {

	m_total.setSize({size, slider_height_v});
	m_current.setSize({size, slider_height_v});

	m_total.setFillColor(colors::navy_blue);
	m_current.setFillColor(colors::blue);

	m_border.setPosition(position);
	m_border.setFillColor(sf::Color::Transparent);
	m_border.setOutlineThickness(2.f);
	m_current.setPosition(position);
	m_total.setPosition(position);

	set_geometry();
}

void Slider::handle_input(input::InputSystem& input, audio::Soundboard& soundboard) {
	if (!input.is_mouse_active()) { return; }
	auto const& mouse = input.get_mouse();

	if (mouse.left_button.released) { m_state = SliderState::neutral; }
	auto const hovered = m_puck.getGlobalBounds().contains(input.get_mouse_position()) || m_total.getGlobalBounds().contains(input.get_mouse_position());
	if (hovered && m_state != SliderState::engaged) {
		if (m_state == SliderState::neutral) { soundboard.play_sound("menu_shift"); }
		m_state = SliderState::hovered;
		if (mouse.left_button.triggered) { m_state = SliderState::engaged; }
	} else {
		if (m_state != SliderState::engaged) { m_state = SliderState::neutral; }
	}
	if (mouse.left_button.held && m_state == SliderState::engaged) {
		auto const width = std::clamp(mouse.position.x - m_total.getGlobalBounds().position.x, 0.f, m_size);
		m_current.setSize({width, slider_height_v});
		m_output = width / m_size;
	}
}

void Slider::update(float const percentage) {
	auto const normalized = std::clamp(percentage, 0.f, 1.f);

	if (m_state != SliderState::engaged) { m_current.setSize({m_total.getSize().x * normalized, m_total.getSize().y}); }

	m_puck.setPosition(m_current.getGlobalBounds().position + sf::Vector2f{m_current.getLocalBounds().size.x, slider_height_v * 0.5f});

	set_geometry();
}

void Slider::render(sf::RenderTarget& win) {
	win.draw(m_border);
	win.draw(m_total);
	win.draw(m_current);
	win.draw(m_puck);
}

void Slider::set_position(sf::Vector2f const position) {
	m_border.setPosition(position);
	m_current.setPosition(position);
	m_total.setPosition(position);
}

void Slider::set_geometry() {
	auto const size = m_state == SliderState::neutral ? 8.f : m_state == SliderState::hovered ? 10.f : 6.f;
	m_puck.setSize({size, size});
	m_puck.setOrigin(m_puck.getLocalBounds().getCenter());
	auto const color = m_state == SliderState::neutral ? colors::ice_blue : m_state == SliderState::hovered ? colors::ui_white : colors::ui_white;
	m_puck.setFillColor(color);

	auto const ccolor = m_state == SliderState::neutral ? colors::blue : m_state == SliderState::hovered ? colors::ice_blue : colors::ice_blue;
	m_current.setFillColor(ccolor);
}

} // namespace fornani::gui
