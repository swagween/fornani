
#pragma once
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include <optional>
#include <memory>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {

class Firefly {
  public:
	Firefly(automa::ServiceProvider& svc, sf::Vector2<float> start);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
  private:
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	anim::AnimatedSprite sprite;
	util::Cooldown light{};
	int variant{};
	bool glowing{};
	std::optional<std::unique_ptr<flfx::SpriteHistory>> trail{};
};

} // namespace vfx
