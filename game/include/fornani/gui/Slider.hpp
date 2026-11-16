
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>

namespace fornani::gui {

class Slider {
  public:
	explicit Slider(sf::Vector2f const position, float const size);
	void handle_input(input::InputSystem& input);
	void update(float const percentage);
	void render(sf::RenderTarget& win);

  private:
	sf::RectangleShape m_border{};
	sf::RectangleShape m_current{};
	sf::RectangleShape m_total{};
	sf::RectangleShape m_puck{};
};

} // namespace fornani::gui
