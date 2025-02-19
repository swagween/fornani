
#pragma once
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"

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

class Dragonfly {
  public:
	Dragonfly(automa::ServiceProvider& svc, sf::Vector2<float> start);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_forces(float seek, float walk, float evade);
	void debug();
  private:
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	anim::AnimatedSprite sprite;
	int variant{};
	struct {
		float seek{};
		float walk{};
		float evade{};
	} forces{};
};

} // namespace fornani::vfx
