
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::components {
class PhysicsComponent;

class SteeringBehavior {
  public:
	void smooth_random_walk(PhysicsComponent& physics, float dampen = 0.005f, float radius = 24.f);
	void smooth_random_walk(PhysicsComponent& physics, HV axis, float dampen = 0.005f, float radius = 24.f);
	void target(PhysicsComponent& physics, sf::Vector2f point, float strength = 0.0001f);
	void seek(PhysicsComponent& physics, sf::Vector2f point, float strength = 0.01f);
	void evade(PhysicsComponent& physics, sf::Vector2f point, float strength = 0.01f, float max_force = 8.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	sf::Vector2f calculate_random_walk(PhysicsComponent& physics, float dampen, float radius);

  private:
	float wander_displacement{};
	sf::Vector2f wander{};
	float wander_radius{};
};

} // namespace fornani::components
