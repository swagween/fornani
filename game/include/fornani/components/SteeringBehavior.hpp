
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

struct ThrustParameters {
	float thrust_power{0.00005f};
	float turn_rate{0.05f};
	float damping{0.999f};
	float arrival_radius{60.f};
};

struct PursuitParameters {
	float trailing_distance{100.f};
	float prediction_time{0.5f};
	float stiffness{0.0001f};
	float damping{0.01f};
	float max_force{0.0001f};
	float wander_strength{0.0005f};
	float wander_radius{60.f};
	float wander_speed{1.f};
	float dead_zone{60.f};
};

} // namespace fornani

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
	void inertial_seek(components::PhysicsComponent& physics, sf::Vector2f point, float max_speed, float max_force);
	void spring_seek(components::PhysicsComponent& physics, sf::Vector2f point, float stiffness, float damping);
	void thrust_seek(components::PhysicsComponent& physics, sf::Vector2f point, ThrustParameters params);
	void arrive(components::PhysicsComponent& physics, sf::Vector2f point, ThrustParameters params);
	void pursue(components::PhysicsComponent& physics, sf::Vector2f target_position, sf::Vector2f target_velocity, sf::Vector2f target_forward, PursuitParameters const& params);

	void evade(PhysicsComponent& physics, sf::Vector2f point, float strength = 0.01f, float max_force = 8.f);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	sf::Vector2f calculate_random_walk(PhysicsComponent& physics, float dampen, float radius);

  private:
	float wander_displacement{};
	float wander_radius{};
	sf::Vector2f m_wander_offset{};
	sf::Vector2f m_wander_target{};
};

} // namespace fornani::components
