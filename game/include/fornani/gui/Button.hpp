
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/MenuTheme.hpp>

namespace fornani::gui {

enum class ButtonState : std::uint8_t { neutral, hovered, clicked };

class Button {
  public:
	Button(automa::ServiceProvider& svc, MenuTheme& theme, std::string_view lbl, sf::Vector2f pos);
	void update(input::InputSystem& input);
	void render(sf::RenderWindow& win);

	[[nodiscard]] auto is_neutral() const -> bool { return m_state == ButtonState::neutral; }
	[[nodiscard]] auto is_hovered() const -> bool { return m_state == ButtonState::hovered; }
	[[nodiscard]] auto is_clicked() const -> bool { return m_state == ButtonState::clicked; }

  private:
	sf::Text m_label;
	ButtonState m_state{};
	MenuTheme* m_theme;
};

} // namespace fornani::gui
