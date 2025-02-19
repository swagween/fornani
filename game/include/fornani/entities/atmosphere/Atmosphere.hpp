#pragma once

#include <vector>
#include <memory>
#include "fornani/entities/atmosphere/Firefly.hpp"
#include "fornani/entities/atmosphere/Dragonfly.hpp"

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::world {
class Map;
} // namespace fornani::world

namespace fornani::player {
class Player;
} // namespace fornani::player

namespace fornani::vfx {
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
} // namespace fornani::npc
