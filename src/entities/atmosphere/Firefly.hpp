
#pragma once
#pragma once

#include <SFML/Graphics.hpp>
#include "../../components/PhysicsComponent.hpp"
#include "../../components/SteeringBehavior.hpp"
#include "../animation/AnimatedSprite.hpp"
#include "../../utils/Cooldown.hpp"

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace vfx {

class Firefly {
  public:
	Firefly(automa::ServiceProvider& svc, sf::Vector2<float> start);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	anim::AnimatedSprite sprite{};
	util::Cooldown light{};
	int variant{};
	bool glowing{};
};

} // namespace vfx
