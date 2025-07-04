#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Direction.hpp"
#include "fornani/weapon/Projectile.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::shape {
class Collider;
class Shape;
} // namespace fornani::shape

namespace fornani::entity {

class Caution {
  public:
	Caution() = default;
	void update();
	void avoid_player(player::Player& player);
	void avoid_ledges(world::Map& map, shape::Collider& collider, Direction& direction, int height);
	Direction projectile_detected(world::Map& map, shape::Shape& zone, arms::Team friendly_fire);
	bool detected_step(world::Map& map, shape::Collider& collider, Direction& direction, sf::Vector2f offset = {}, int vision = 1);

	void debug_render(sf::RenderWindow& win, sf::Vector2<float> cam);

	[[nodiscard]] bool danger() const;

	sf::Vector2<float> retreat{};
	struct {
		sf::Vector2<float> left{};
		sf::Vector2<float> right{};
	} testers{};
	struct {
		int perceived{};
		int danger{};
	} heights{};

  private:
	io::Logger m_logger{"Enemy"};
};

} // namespace fornani::entity
