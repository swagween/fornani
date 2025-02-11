#include "fornani/components/SteeringBehavior.hpp"
#include <algorithm>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::components {

void SteeringBehavior::smooth_random_walk(PhysicsComponent& physics, float dampen, float radius) {
	wander = physics.position + util::unit(physics.velocity) * (radius + radius / 3.f);
	wander_radius = radius;
	wander_displacement += util::Random::random_range_float(-0.08f, 0.08f);
	auto const theta = wander_displacement + atan2f(physics.velocity.y, physics.velocity.x);
	auto const x = radius * cos(theta);
	auto const y = radius * sin(theta);
	auto const target = wander + sf::Vector2{x, y};
	auto const steering = util::unit(target - physics.position) * dampen;
	physics.apply_force(steering);
}

void SteeringBehavior::target(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength) {
	auto distance = point - physics.position;
	auto mag = util::magnitude(distance);
	auto epsilon{0.1f};
	if (abs(mag) < epsilon) { physics.position = point; }
	physics.apply_force(distance * strength);
}

void SteeringBehavior::seek(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength) {
	auto force = point - physics.position;
	force -= physics.velocity;
	force *= strength;
	physics.apply_force(force);
}

void SteeringBehavior::evade(components::PhysicsComponent& physics, sf::Vector2<float> point, float strength, float max_force) {
	auto distance = point - physics.position;
	auto mag = util::magnitude(distance);
	mag = std::max(0.0001f, mag);
	auto str = strength / (mag * mag);
	str = std::clamp(str, 0.f, max_force);
	physics.apply_force(distance * str * -1.f);
}

void SteeringBehavior::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sf::CircleShape wander_circle{};
	wander_circle.setRadius(2.f);
	wander_circle.setOrigin({2.f, 2.f});
	wander_circle.setOutlineColor(svc.styles.colors.goldenrod);
	wander_circle.setFillColor(sf::Color::Transparent);
	wander_circle.setOutlineThickness(-2);
	wander_circle.setPosition(wander - cam);
	win.draw(wander_circle);
	wander_circle.setRadius(wander_radius);
	wander_circle.setOrigin({wander_radius, wander_radius});
	wander_circle.setOutlineColor(svc.styles.colors.green);
	wander_circle.setOutlineThickness(-1);
	wander_circle.setPointCount(32);
	win.draw(wander_circle);
}

} // namespace fornani::components
