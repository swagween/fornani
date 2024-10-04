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
struct Direction;
}

namespace entity {

class Caution {
  public:
	Caution() = default;
	void update();
	void avoid_player(player::Player& player);
	void avoid_ledges(world::Map& map, shape::Collider& collider, dir::Direction& direction, int height);
	bool danger() const;

	sf::Vector2<float> retreat{};
	struct {
		sf::Vector2<float> left{};
		sf::Vector2<float> right{};
	} testers{};
	struct {
		int perceived{};
		int danger{};
	} heights{};
};

} // namespace entity