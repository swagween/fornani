
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::components {

static float wrap_angle(float a) {
	while (a > std::numbers::pi_v<float>) a -= 2.f * std::numbers::pi_v<float>;
	while (a < -std::numbers::pi_v<float>) a += 2.f * std::numbers::pi_v<float>;
	return a;
}

static float angle_to_target(sf::Vector2f from, sf::Vector2f to) { return std::atan2(to.y - from.y, to.x - from.x); }

void SteeringBehavior::smooth_random_walk(PhysicsComponent& physics, float dampen, float radius) { physics.apply_force(calculate_random_walk(physics, dampen, radius)); }

void SteeringBehavior::smooth_random_walk(PhysicsComponent& physics, HV axis, float dampen, float radius) {
	auto force = calculate_random_walk(physics, dampen, radius);
	switch (axis) {
	case HV::horizontal: force.y = 0.f; break;
	case HV::vertical: force.x = 0.f; break;
	default: break;
	}
	physics.apply_force(force);
}

void SteeringBehavior::target(components::PhysicsComponent& physics, sf::Vector2f point, float strength) {
	auto distance = point - physics.position;
	auto mag = util::magnitude(distance);
	auto epsilon{0.1f};
	if (std::abs(mag) < epsilon) { physics.position = point; }
	physics.apply_force(distance * strength);
}

void SteeringBehavior::seek(components::PhysicsComponent& physics, sf::Vector2f point, float strength) {
	auto force = point - physics.position;
	force -= physics.velocity;
	force *= strength;
	physics.apply_force(force);
}

void SteeringBehavior::inertial_seek(components::PhysicsComponent& physics, sf::Vector2f point, float max_speed, float max_force) {

	sf::Vector2f toTarget = point - physics.position;

	float distance = std::sqrt(toTarget.x * toTarget.x + toTarget.y * toTarget.y);
	if (distance < constants::tiny_value) { return; }

	// Normalize
	sf::Vector2f desired = toTarget / distance;

	// 🔑 Scale speed based on distance
	float speed = max_speed;
	if (distance < 100.f) speed *= (distance / 100.f);

	desired *= speed;

	sf::Vector2f steering = desired - physics.velocity;

	// Clamp force
	float mag = std::sqrt(steering.x * steering.x + steering.y * steering.y);
	if (mag > max_force) steering = (steering / mag) * max_force;

	physics.apply_force(steering);
}

void SteeringBehavior::spring_seek(components::PhysicsComponent& physics, sf::Vector2f point, float stiffness, float damping) {
	sf::Vector2f displacement = point - physics.position;
	sf::Vector2f force = displacement * stiffness - physics.velocity * damping;
	physics.apply_force(force);
}

void SteeringBehavior::thrust_seek(components::PhysicsComponent& physics, sf::Vector2f point, ThrustParameters params) {
	sf::Vector2f to_target = point - physics.position;
	float distance = to_target.length();

	if (distance < constants::tiny_value) { return; }

	sf::Vector2f desired_dir = to_target.normalized();

	float jitter = 0.02f;
	sf::Vector2f random_offset{random::random_range_float(-1.f, 1.f), random::random_range_float(-1.f, 1.f)};
	desired_dir = (desired_dir + random_offset * jitter).normalized();

	if (physics.velocity.length() < constants::tiny_value) { physics.velocity = random::random_vector_float(-constants::small_value, constants::small_value); }
	sf::Vector2f forward = physics.velocity.normalized();

	if (forward.length() < constants::tiny_value) { forward = desired_dir; }

	if (physics.actual_speed() < constants::tiny_value) {
		forward = desired_dir;
	} else {
		forward = physics.velocity / physics.actual_speed();
		forward = (forward + (desired_dir - forward) * params.turn_rate).normalized();
	}

	float alignment = util::dot(forward, desired_dir);

	float thrust_scale = 1.0f;
	if (distance < params.arrival_radius) thrust_scale = distance / params.arrival_radius;

	if (alignment > 0.7f) {
		float final_thrust = params.thrust_power * alignment * thrust_scale;
		physics.apply_force(forward * final_thrust);
	}

	physics.velocity *= params.damping;
}

void SteeringBehavior::evade(components::PhysicsComponent& physics, sf::Vector2f point, float strength, float max_force) {
	auto distance = point - physics.position;
	auto mag = util::magnitude(distance);
	mag = std::max(0.0001f, mag);
	auto str = strength / (mag * mag);
	str = std::clamp(str, 0.f, max_force);
	physics.apply_force(distance * str * -1.f);
}

void SteeringBehavior::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	sf::CircleShape wander_circle{};
	wander_circle.setRadius(2.f);
	wander_circle.setOrigin({2.f, 2.f});
	wander_circle.setOutlineColor(colors::goldenrod);
	wander_circle.setFillColor(sf::Color::Transparent);
	wander_circle.setOutlineThickness(-2);
	wander_circle.setPosition(wander - cam);
	win.draw(wander_circle);
	wander_circle.setRadius(wander_radius);
	wander_circle.setOrigin({wander_radius, wander_radius});
	wander_circle.setOutlineColor(colors::green);
	wander_circle.setOutlineThickness(-1);
	wander_circle.setPointCount(32);
	win.draw(wander_circle);
}

sf::Vector2f SteeringBehavior::calculate_random_walk(PhysicsComponent& physics, float dampen, float radius) {
	wander = physics.position + util::unit(physics.velocity) * (radius + radius / 3.f);
	wander_radius = radius;
	wander_displacement += random::random_range_float(-0.08f, 0.08f);
	float const theta = wander_displacement + ::std::atan2(physics.velocity.y, physics.velocity.x);
	float const x = radius * ::std::cos(theta);
	float const y = radius * ::std::sin(theta);
	auto const target = wander + sf::Vector2f{x, y};
	auto const steering = util::unit(target - physics.position) * dampen;
	return steering;
}

} // namespace fornani::components
