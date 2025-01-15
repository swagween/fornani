#include "SteeringBehavior.hpp"
#include "PhysicsComponent.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Math.hpp"
#include <algorithm>

namespace components {

void SteeringBehavior::smooth_random_walk(automa::ServiceProvider& svc, components::PhysicsComponent& physics, float dampen, float radius) {
	wander = physics.position + util::unit(physics.velocity) * (radius + radius / 3.f);
	wander_radius = radius;
	wander_displacement += svc.random.random_range_float(-0.08f, 0.08f);
	auto theta = wander_displacement + atan2f(physics.velocity.y, physics.velocity.x);
	auto x = radius * cos(theta);
	auto y = radius * sin(theta);
	auto target = wander + sf::Vector2<float>{x, y};
	auto steering = util::unit(target - physics.position) * dampen;
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
	auto epsilon{0.1f};
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

} // namespace components
