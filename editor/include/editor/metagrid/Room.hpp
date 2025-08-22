
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/graphics/Colors.hpp>

namespace pi {

constexpr auto spacing_v = 16.f;
constexpr sf::Color excluded_room_color_v{120, 80, 80, 20};
constexpr sf::Color highlighted_excluded_room_color_v{120, 100, 80, 60};
constexpr sf::Color room_color_v{80, 120, 180, 60};
constexpr sf::Color highighted_room_color_v{60, 130, 190, 90};

class Room {
  public:
	Room(dj::Json const& in);

	void update(sf::Vector2f mouse_position) { m_highlighted = m_box.getGlobalBounds().contains(mouse_position); }

	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void toggle_minimap() { m_include_in_minimap = !m_include_in_minimap; }

	[[nodiscard]] auto get_board_position() const -> sf::Vector2f { return sf::Vector2f{m_position} * spacing_v; }
	[[nodiscard]] auto is_highlighted() const -> bool { return m_highlighted; }

  private:
	sf::RectangleShape m_box{};
	sf::Vector2i m_position{};
	bool m_highlighted{};
	bool m_include_in_minimap{};
};

} // namespace pi
