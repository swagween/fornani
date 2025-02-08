
#pragma once

#include <SFML/Graphics.hpp>

namespace automa {
struct ServiceProvider;
}

namespace graphics {

class DayNightShifter {
  public:
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr, std::uint8_t const native_alpha);
};

} // namespace graphics
