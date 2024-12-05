
#pragma once
#pragma once

#include <SFML/Graphics.hpp>

namespace automa {
struct ServiceProvider;
}

namespace vfx {

class Firefly {
  public:
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	sf::Vector2<float> position{};
};

} // namespace vfx
