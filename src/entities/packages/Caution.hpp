#pragma once
#include <SFML/Graphics.hpp>

namespace player {
class Player;
}

namespace world {
class Map;
}

namespace shape {
class Collider;
}

namespace dir {
class Direction;
}

namespace entity {

class Caution {
  public:
	Caution() = default;
	void update();
	void avoid_player(player::Player& player);
	void avoid_ledges(world::Map& map, shape::Collider& collider, int height);
	bool danger(dir::Direction& direction) const;

	sf::Vector2<float> retreat{};
};

} // namespace entity