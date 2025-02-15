
#pragma once

#include <SFML/Graphics.hpp>

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::components {
class PhysicsComponent;

class SteeringBehavior {
  public:
	void smooth_random_walk(automa::ServiceProvider& svc, PhysicsComponent& physics, float dampen = 0.005f, float radius = 24.f);
	void target(PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.0001f);
	void seek(PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.01f);
	void evade(PhysicsComponent& physics, sf::Vector2<float> point, float strength = 0.01f, float max_force = 8.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	private:
	float wander_displacement{};
	sf::Vector2<float> wander{};
	float wander_radius{};
};

} // namespace fornani::components
