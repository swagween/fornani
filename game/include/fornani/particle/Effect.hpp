
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/entities/Entity.hpp"
#include "fornani/entities/animation/Animation.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::entity {

class Effect final : public Entity {
  public:
	Effect(automa::ServiceProvider& svc, std::string const& label, sf::Vector2<float> pos, sf::Vector2<float> vel, int type = 0, int index = 0, sf::Vector2i reflections = {});
	void update(automa::ServiceProvider& svc, world::Map& map) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void rotate();
	[[nodiscard]] auto done() -> bool { return animation.complete(); }

  private:
	anim::Animation animation{};
	components::PhysicsComponent physics{};
	int type{};
	sf::Sprite sprite;
};

} // namespace fornani::entity
