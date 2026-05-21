
#pragma once
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <optional>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani {
class Renderer;
}

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::world {
class Map;
} // namespace fornani::world

namespace fornani::vfx {

class Firefly {
  public:
	Firefly(automa::ServiceProvider& svc, sf::Vector2f start);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void submit(Renderer& renderer);
	std::optional<graphics::SpriteHistory> trail{};

  private:
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	anim::AnimatedSprite sprite;
	util::Cooldown light{};
	int variant{};
	bool glowing{};
};

} // namespace fornani::vfx
