
#pragma once

#include <SFML/Graphics.hpp>

namespace automa {
struct ServiceProvider;
}

namespace components {
class PhysicsComponent;

class SteeringBehavior {
  public:
	void smooth_random_walk(automa::ServiceProvider& svc, components::PhysicsComponent& physics, float dampen = 0.005f, float radius = 24.f);
	void target(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.0001f);
	void seek(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.01f);
	void evade(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.01f, float max_force = 8.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	private:
	float wander_displacement{};
	sf::Vector2<float> wander{};
	float wander_radius{};
};

} // namespace components
