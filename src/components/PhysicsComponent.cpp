#include "PhysicsComponent.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace components {

void PhysicsComponent::apply_force(sf::Vector2<float> force) { sf::operator+=(acceleration, force); }

void PhysicsComponent::apply_force_at_angle(float magnitude, float angle) {
	acceleration.x += (magnitude * cos(angle)) / mass;
	acceleration.y += (magnitude * sin(angle)) / mass;
}

void PhysicsComponent::multiply_velocity(float multiplier) {
	velocity *= multiplier;
	real_velocity *= multiplier;
}

void PhysicsComponent::multiply_acceleration(float multiplier, sf::Vector2<float> weight) {
	acceleration.x *= weight.x > 0.f ? multiplier * weight.x : 1.f;
	acceleration.y *= weight.y > 0.f ? multiplier * weight.y : 1.f;
}

void PhysicsComponent::update_euler(automa::ServiceProvider& svc) {
	integrate(svc);
	direction.und = velocity.y > 0.f ? dir::UND::down : (velocity.y < 0.f ? dir::UND::up : dir::UND::neutral);
	direction.lr = velocity.x > 0.f ? dir::LR::right : (velocity.x < 0.f ? dir::LR::left : dir::LR::neutral);
}

void PhysicsComponent::integrate(automa::ServiceProvider& svc) {

	auto dt = svc.ticker.global_tick_rate();
	previous_acceleration = acceleration;
	previous_velocity = velocity;
	previous_position = position;

	acceleration.y += gravity * dt;
	sf::Vector2<float> friction = flags.test(State::grounded) ? ground_friction : air_friction;
	velocity.x = (velocity.x + (acceleration.x / mass) * dt) * friction.x;
	velocity.y = (velocity.y + (acceleration.y / mass) * dt) * friction.y;
	velocity.x = std::clamp(velocity.x, -maximum_velocity.x, maximum_velocity.x);
	velocity.y = std::clamp(velocity.y, -maximum_velocity.y, maximum_velocity.y);
	position = position + velocity * dt;
	real_velocity = velocity * dt;
}

void PhysicsComponent::update(automa::ServiceProvider& svc) { update_euler(svc); }

void PhysicsComponent::update_dampen(automa::ServiceProvider& svc) {
	update_euler(svc);
	acceleration = {};
}

void PhysicsComponent::simple_update() {
	velocity *= air_friction.x;
	velocity += acceleration;
	position += velocity;
	acceleration = {};
}

void PhysicsComponent::impart_momentum() {
	position += forced_momentum;
	velocity += forced_acceleration;
}

void PhysicsComponent::hard_stop_x() {
	velocity.x = 0.f;
	real_velocity.x = 0.f;
}

void PhysicsComponent::stop_x() {
	acceleration.x = 0.f;
}

void PhysicsComponent::zero() {
	acceleration = {};
	velocity = {};
	real_velocity = {};
}

void PhysicsComponent::zero_x() {
	acceleration.x = 0.0f;
	velocity.x *= -elasticity;
	real_velocity.x *= -elasticity;
}

void PhysicsComponent::zero_y() {
	acceleration.y = 0.0f;
	velocity.y *= -elasticity;
	real_velocity.y *= -elasticity;
}

void PhysicsComponent::hitstun() {}

void PhysicsComponent::set_constant_friction(sf::Vector2<float> fric) {
	ground_friction = {fric.x, fric.x};
	air_friction = {fric.y, fric.y};
}

void PhysicsComponent::set_global_friction(float fric) {
	ground_friction = {fric, fric};
	air_friction = {fric, fric};
}

} // namespace components
