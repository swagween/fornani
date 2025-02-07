#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Counter.hpp"
#include "fornani/entities/animation/Animation.hpp"

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

enum class SpawnerAttributes { };

class Spawner {
  public:
	Spawner(automa::ServiceProvider& svc, sf::Vector2<float> position, int enemy_id = 0);
	void update(automa::ServiceProvider& svc, Map& map);
	void set_id(int id) { enemy_id = id; }

  private:
	sf::Vector2<float> position{};
	util::BitFlags<SpawnerAttributes> attributes{};
	int enemy_id{};
	util::Cooldown spawn_timer{512};

};
} // namespace world