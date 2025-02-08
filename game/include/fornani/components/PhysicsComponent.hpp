
#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <random>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Direction.hpp"

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::components {

constexpr sf::Vector2 FRICTION_DEFAULT = {0.9f, 0.9f};
constexpr float MASS_DEFAULT = 1.0f;
constexpr float UNIVERSAL_MAX_SPEED = 64.0f;
constexpr float TERMINAL_VELOCITY = 1.8f;
constexpr float default_grav = 1.0f;
constexpr float elastic_threshold{-0.01f};

enum class State { grounded };

class PhysicsComponent {
  public:
	PhysicsComponent() : ground_friction(FRICTION_DEFAULT), air_friction(FRICTION_DEFAULT), mass(MASS_DEFAULT){};
	PhysicsComponent(sf::Vector2<float> fric, float ma, sf::Vector2<float> max_vel = sf::Vector2{UNIVERSAL_MAX_SPEED, UNIVERSAL_MAX_SPEED}, float grav = 0.0f)
		: ground_friction(fric), air_friction(fric), mass(ma), gravity(grav), maximum_velocity(max_vel) {}

	// basic physics variables
	sf::Vector2<float> acceleration{};
	sf::Vector2<float> velocity{};
	sf::Vector2<float> real_velocity{};
	sf::Vector2<float> position{};
	sf::Vector2<float> ground_friction{};
	sf::Vector2<float> air_friction{};
	sf::Vector2<float> forced_momentum{};
	sf::Vector2<float> forced_acceleration{};
	sf::Vector2<float> mtv{};

	sf::Vector2<float> previous_acceleration{};
	sf::Vector2<float> previous_velocity{};
	sf::Vector2<float> previous_position{};

	// properties
	float mass{1.0f};
	float elasticity{0.0f};
	float gravity{};
	sf::Vector2<float> maximum_velocity{UNIVERSAL_MAX_SPEED, UNIVERSAL_MAX_SPEED};

	void apply_force(sf::Vector2<float> force);
	void apply_force_at_angle(float magnitude, float angle);
	void multiply_velocity(float multiplier);
	void multiply_acceleration(float multiplier, sf::Vector2<float> weight);
	void update_euler(automa::ServiceProvider& svc);
	void integrate(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc);
	void update_dampen(automa::ServiceProvider& svc);
	void simple_update();
	void impart_momentum();
	void hard_stop_x();
	void stop_x();
	void zero();
	void zero_x();
	void zero_y();
	void hitstun();
	void set_constant_friction(sf::Vector2<float> fric);
	void set_global_friction(float fric);
	[[nodiscard]] auto actual_velocity() const -> sf::Vector2<float> { return position - previous_position; }
	[[nodiscard]] auto apparent_velocity() const -> sf::Vector2<float> { return real_velocity; }
	[[nodiscard]] auto apparent_acceleration() const -> sf::Vector2<float> { return real_velocity - previous_velocity; }
	[[nodiscard]] auto actual_speed() const -> float { return actual_velocity().length(); }
	[[nodiscard]] auto elastic_collision() const -> bool { return velocity.x * previous_velocity.x < elastic_threshold || velocity.y * previous_velocity.y < elastic_threshold; }
	[[nodiscard]] auto stationary() const -> bool { return abs(velocity.x) < epsilon && abs(velocity.y) < epsilon; }

	util::BitFlags<State> flags{};
	dir::Direction direction{};
	
	float epsilon{0.0001f};
};

} // namespace fornani::components
