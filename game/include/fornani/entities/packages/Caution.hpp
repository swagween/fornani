#pragma once
#include <SFML/Graphics.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::shape {
class Collider;
}

namespace fornani::dir {
struct Direction;
}

namespace fornani::entity {

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