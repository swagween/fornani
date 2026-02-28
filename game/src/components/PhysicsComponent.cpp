
#include <ccmath/ext/clamp.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <cmath>

namespace fornani::components {

void PhysicsComponent::apply_force(sf::Vector2f force) { sf::operator+=(acceleration, force); }

void PhysicsComponent::apply_force_at_angle(float magnitude, float angle) {
	acceleration.x += (magnitude * std::cos(angle)) / mass;
	acceleration.y += (magnitude * std::sin(angle)) / mass;
}

void PhysicsComponent::multiply_velocity(float multiplier) {
	velocity *= multiplier;
	real_velocity *= multiplier;
}

void PhysicsComponent::multiply_acceleration(float multiplier, sf::Vector2f weight) {
	acceleration.x *= weight.x > 0.f ? multiplier * weight.x : 1.f;
	acceleration.y *= weight.y > 0.f ? multiplier * weight.y : 1.f;
}

void PhysicsComponent::update_euler(automa::ServiceProvider& svc) {
	integrate(svc);
	direction.und = velocity.y > 0.f ? UND::down : (velocity.y < 0.f ? UND::up : UND::neutral);
	direction.lnr = velocity.x > 0.f ? LNR::right : (velocity.x < 0.f ? LNR::left : LNR::neutral);
}

void PhysicsComponent::integrate(automa::ServiceProvider& svc) {

	auto dt = svc.ticker.global_tick_rate();
	previous_acceleration = acceleration;
	previous_velocity = velocity;
	previous_position = position;

	acceleration.y += gravity * dt;
	sf::Vector2f friction = flags.test(State::grounded) ? ground_friction : air_friction;
	velocity = (velocity + (acceleration / mass) * dt).componentWiseMul(friction);
	velocity.x = ccm::ext::clamp(velocity.x, -maximum_velocity.x, maximum_velocity.x);
	velocity.y = ccm::ext::clamp(velocity.y, -maximum_velocity.y, maximum_velocity.y);
	position = position + velocity * dt;
	real_velocity = velocity * dt;
}

void PhysicsComponent::update(automa::ServiceProvider& svc) { update_euler(svc); }

void PhysicsComponent::update_dampen(automa::ServiceProvider& svc) {
	update_euler(svc);
	acceleration = {};
}

void PhysicsComponent::simple_update(bool gravity) {
	if (gravity) { acceleration.y = gravity; }
	velocity *= air_friction.x;
	velocity += acceleration;
	position += velocity;
	acceleration = {};
}

void PhysicsComponent::impart_momentum() {
	position += forced_momentum;
	velocity += forced_acceleration;
	if (!flags.test(State::grounded)) { forced_momentum = forced_momentum.componentWiseMul(air_friction); }
}

void PhysicsComponent::hard_stop_x() {
	velocity.x = 0.f;
	real_velocity.x = 0.f;
}

void PhysicsComponent::stop_x() { acceleration.x = 0.f; }

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

void PhysicsComponent::ricochet_vertically() {
	acceleration.y = 0.0f;
	velocity.y = maximum_velocity.y;
	real_velocity.y = maximum_velocity.y;
}

void PhysicsComponent::hitstun() {}

void PhysicsComponent::set_friction_componentwise(sf::Vector2f fric) {
	ground_friction = fric;
	air_friction = fric;
}

void PhysicsComponent::adopt(PhysicsComponent& other) {
	velocity = other.velocity;
	real_velocity = other.real_velocity;
	acceleration = other.acceleration;
	forced_acceleration = other.forced_acceleration;
	forced_momentum = other.forced_momentum;
}

void PhysicsComponent::set_constant_friction(sf::Vector2f fric) {
	ground_friction = {fric.x, fric.x};
	air_friction = {fric.y, fric.y};
}

void PhysicsComponent::set_global_friction(float fric) {
	ground_friction = {fric, fric};
	air_friction = {fric, fric};
}

void PhysicsComponent::collide(sf::Vector2i direction) {
	if (direction.x == 1) {
		acceleration.x *= -elasticity;
		velocity.x *= -elasticity;
		real_velocity.x *= -elasticity;
	}
	if (direction.y == 1) {
		acceleration.y *= -elasticity;
		velocity.y *= -elasticity;
		real_velocity.y *= -elasticity;
	}
}

} // namespace fornani::components
