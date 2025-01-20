#pragma once

#include <vector>
#include <memory>
#include "fornani/entities/atmosphere/Firefly.hpp"
#include "fornani/entities/atmosphere/Dragonfly.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace player {
class Player;
}

namespace vfx {
class Atmosphere {
  public:
	Atmosphere(automa::ServiceProvider& svc, sf::Vector2<float> span, int type);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void debug();

  private:
	std::vector<std::unique_ptr<Firefly>> fireflies{};
	std::vector<Dragonfly> dragonflies{};

	// debug
	struct {
		float seek{};
		float walk{};
		float evade{};
	} forces{};
};
} // namespace npc
