
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/graphics/Animatable.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::entity {

class Effect final : public Animatable {
  public:
	Effect(automa::ServiceProvider& svc, std::string const& label, sf::Vector2f pos, sf::Vector2f vel = {}, int channel = 0);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void rotate();
	[[nodiscard]] auto done() -> bool { return animation.complete(); }

  private:
	components::PhysicsComponent physics{};
};

} // namespace fornani::entity
