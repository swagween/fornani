
#include <fornani/gui/Button.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

Button::Button(automa::ServiceProvider& svc, MenuTheme& theme, std::string_view lbl, sf::Vector2f pos) : m_label{svc.text.fonts.title.font}, m_theme(&theme) {
	m_label.setString(lbl.data());
	m_label.setPosition(pos);
	m_label.setCharacterSize(16);
	m_label.setLetterSpacing(1.f);
	m_label.setFillColor(theme.primary_text_color);
}

void Button::update(input::InputSystem& input) {
	m_state = ButtonState::neutral;
	if (m_label.getGlobalBounds().contains(input.get_mouse_position())) {
		m_state = ButtonState::hovered;
		if (input.left_clicked() && input.is_mouse_active()) { m_state = ButtonState::clicked; }
	}
	is_hovered() ? m_label.setFillColor(m_theme->primary_text_color) : m_label.setFillColor(m_theme->secondary_text_color);
}

void Button::render(sf::RenderWindow& win) { win.draw(m_label); }

} // namespace fornani::gui
