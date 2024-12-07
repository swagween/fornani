#pragma once

#include <vector>
#include "Firefly.hpp"
#include "Dragonfly.hpp"

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
	std::vector<Firefly> fireflies{};
	std::vector<Dragonfly> dragonflies{};

	// debug
	struct {
		float seek{};
		float walk{};
		float evade{};
	} forces{};
};
} // namespace npc
