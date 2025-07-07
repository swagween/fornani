#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/Collider.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

enum class SpawnerAttributes {};

class Spawner {
  public:
	Spawner(automa::ServiceProvider& svc, sf::Vector2f position, int enemy_id = 0);
	void update(automa::ServiceProvider& svc, Map& map);
	void set_id(int const id) { enemy_id = id; }

  private:
	sf::Vector2f position{};
	util::BitFlags<SpawnerAttributes> attributes{};
	int enemy_id{};
	util::Cooldown spawn_timer{512};
};
} // namespace fornani::world