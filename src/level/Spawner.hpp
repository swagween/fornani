#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "../setup/EnumLookups.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Counter.hpp"
#include "../entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
class Projectile;
}

namespace world {

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
	util::Counter spawn_count{};
	util::Cooldown spawn_timer{256};

};
} // namespace world