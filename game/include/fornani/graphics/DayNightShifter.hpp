
#pragma once

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::graphics {

class DayNightShifter {
  public:
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr);
};

} // namespace graphics
