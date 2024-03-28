
#pragma once

#include "PhysicsComponent.hpp"
#include "../service/ServiceProvider.hpp"
#include <algorithm>

namespace components {

void PhysicsComponent::apply_force(sf::Vector2<float> force) { sf::operator+=(acceleration, force); }

void PhysicsComponent::apply_force_at_angle(float magnitude, float angle) {
	acceleration.x += (magnitude * cos(angle)) / mass;
	acceleration.y += (magnitude * sin(angle)) / mass;
}

void PhysicsComponent::update_euler(automa::ServiceProvider& svc) {

	integrate(svc);
	direction.und = velocity.y > 0.f ? dir::UND::down : (velocity.y < 0.f ? dir::UND::up : dir::UND::neutral);
	direction.lr = velocity.x > 0.f ? dir::LR::right : (velocity.x < 0.f ? dir::LR::left : dir::LR::neutral);

}

void PhysicsComponent::integrate(automa::ServiceProvider& svc) {

	float dt = svc.ticker.global_tick_rate();
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

	if (y_acc_history.size() < acceleration_sample_size) { y_acc_history.push_back(acceleration.y);
	} else {
		y_acc_history.pop_front();
		y_acc_history.push_back(acceleration.y);
	}
	if (x_acc_history.size() < acceleration_sample_size) {
		x_acc_history.push_back(acceleration.x);
	} else {
		x_acc_history.pop_front();
		x_acc_history.push_back(acceleration.x);
	}

	calculate_maximum_acceleration();
	calculate_jerk();
}

void PhysicsComponent::update(automa::ServiceProvider& svc) { update_euler(svc); }

void PhysicsComponent::update_dampen(automa::ServiceProvider& svc) {
	update_euler(svc);
	acceleration = {};
}

void PhysicsComponent::calculate_maximum_acceleration() {
	float max = acceleration.x;
	for (auto& acc : x_acc_history) {
		if (acc > max) { max = acc; }
	}

}

void PhysicsComponent::calculate_jerk() {
	float sum{};
	float previous_y{acceleration.y};
	for (auto& instance : y_acc_history) {
		sum += instance - previous_y;
		previous_y = instance;
	}
	jerk.y = sum / acceleration_sample_size;
	float previous_x{acceleration.x};
	for (auto& instance : x_acc_history) {
		sum += instance - previous_x;
		previous_x = instance;
	}
	jerk.x = sum / acceleration_sample_size;
}

void PhysicsComponent::zero() {
	acceleration = {0.0f, 0.0f};
	velocity = {0.0f, 0.0f};
}

void PhysicsComponent::zero_x() {
	acceleration.x = 0.0f;
	velocity.x *= -elasticity;
}

void PhysicsComponent::zero_y() {
	acceleration.y = 0.0f;
	velocity.y *= -elasticity;
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
