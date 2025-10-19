#pragma once

#include <memory>
#include <vector>
#include "fornani/entities/atmosphere/Dragonfly.hpp"
#include "fornani/entities/atmosphere/Firefly.hpp"

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
	Atmosphere(automa::ServiceProvider& svc, sf::Vector2f span, int type);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void debug();

  private:
	std::vector<std::unique_ptr<Firefly>> fireflies{};
	std::vector<std::unique_ptr<Dragonfly>> dragonflies{};

	// debug
	struct {
		float seek{};
		float walk{};
		float evade{};
	} forces{};
};
} // namespace fornani::vfx
