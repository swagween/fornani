
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::graphics {

class DayNightShifter {
  public:
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Sprite& sprite, int ctr, std::uint8_t const native_alpha);

  private:
	io::Logger m_logger{"Graphics"};
};

} // namespace fornani::graphics
