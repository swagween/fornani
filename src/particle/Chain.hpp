
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Spring.hpp"

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
class Chain {
  public:
	Chain(SpringParameters params, sf::Vector2<float> position, int num_links, bool reversed = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, float dampen = 1.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_gravity(float g) { grav = g; }
	std::vector<Spring> links{};
	std::vector<components::PhysicsComponent> joints{};
  private:
	sf::Vector2<float> root{};
	float external_dampen{0.05f};
	float grav{1.f};
};

} // namespace vfx
