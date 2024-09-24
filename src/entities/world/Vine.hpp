#pragma once

#include <string>
#include "../../utils/Shape.hpp"
#include "../../particle/Chain.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace player {
class Player;
}

namespace entity {
class Vine {
  public:
	Vine(automa::ServiceProvider& svc, sf::Vector2<float> position, int length = 5);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	sf::Vector2<float> position{};
	sf::Vector2<float> spacing{0.f, 24.f};
	int length{};
	sf::RectangleShape drawbox{};
	vfx::Chain chain;
};

} // namespace entity
