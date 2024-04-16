
#pragma once
#include <vector>
#include <string_view>
#include <SFML/Graphics.hpp>
#include "../utils/Cooldown.hpp"
#include "../entities/Entity.hpp"
#include "../entities/animation/Animation.hpp"
#include "../components/PhysicsComponent.hpp"

namespace automa {
struct ServiceProvider;
}

namespace entity {

class Explosion : public Entity {
	public:
	Explosion(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> vel, int type = 0, int size = 0);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto done() const -> bool { return animation.complete(); }

  private:
	anim::Animation animation{};
	components::PhysicsComponent physics{};
	int type{};
};

} // namespace vfx
