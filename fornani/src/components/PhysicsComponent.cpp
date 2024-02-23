
#pragma once

#include "PhysicsComponent.hpp"
#include "../setup/ServiceLocator.hpp"

namespace components {

using Time = std::chrono::duration<float>;

void PhysicsComponent::apply_force(sf::Vector2<float> force) { sf::operator+=(acceleration, force); }

void PhysicsComponent::apply_force_at_angle(float magnitude, float angle) {
	acceleration.x += (magnitude * cos(angle)) / mass;
	acceleration.y += (magnitude * sin(angle)) / mass;
}

void PhysicsComponent::update_euler() {

	svc::tickerLocator.get().tick([this] { integrate(); });

}

void PhysicsComponent::integrate() {

	float ndt = svc::tickerLocator.get().ft.count() * svc::tickerLocator.get().tick_multiplier;
	previous_acceleration = acceleration;
	previous_velocity = velocity;
	previous_position = position;

	acceleration.y += gravity * ndt;
	velocity.x = (velocity.x + (acceleration.x / mass) * ndt) * friction.x;
	velocity.y = (velocity.y + (acceleration.y / mass) * ndt) * friction.y;
	if (velocity.x > maximum_velocity.x) { velocity.x = maximum_velocity.x; }
	if (velocity.x < -maximum_velocity.x) { velocity.x = -maximum_velocity.x; }
	if (velocity.y > maximum_velocity.y) { velocity.y = maximum_velocity.y; }
	if (velocity.y < -maximum_velocity.y) { velocity.y = -maximum_velocity.y; }
	position = position + velocity * ndt;
	acceleration.y - 0.f;
}

void PhysicsComponent::update() { update_euler(); }

void PhysicsComponent::update_dampen() {
	//acceleration /= svc::tickerLocator.get().tick_rate;
	update_euler();
	acceleration = {0.0f, 0.0f};
}

void PhysicsComponent::zero() {
	acceleration = {0.0f, 0.0f};
	velocity = {0.0f, 0.0f};
}

void PhysicsComponent::hitstun() { dt /= 2.0f; }

} // namespace components
