
#pragma once
#include <vector>
#include <string_view>
#include <SFML/Graphics.hpp>
#include "fornani/utils/Cooldown.hpp"
#include "fornani/entities/Entity.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/components/PhysicsComponent.hpp"

namespace automa {
struct ServiceProvider;
}

namespace entity {

class Effect : public Entity {
	public:
	Effect(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> vel, int type = 0, int index = 0, sf::Vector2i reflections = {});
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void rotate();
	[[nodiscard]] auto done() -> bool { return animation.complete(); }

  private:
	anim::Animation animation{};
	components::PhysicsComponent physics{};
	int type{};
	sf::Sprite sprite;
};

} // namespace vfx
