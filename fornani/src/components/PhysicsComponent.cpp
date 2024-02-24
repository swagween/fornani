
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

	//svc::tickerLocator.get().tick([this] { integrate(); });
	integrate();

}

void PhysicsComponent::integrate() {

	float ndt = svc::tickerLocator.get().ft.count() * svc::tickerLocator.get().tick_multiplier;
	previous_acceleration = acceleration;
	previous_velocity = velocity;
	previous_position = position;

	acceleration.y += gravity * ndt;
	sf::Vector2<float> friction = flags.test(State::grounded) ? ground_friction : air_friction;
	velocity.x = (velocity.x + (acceleration.x / mass) * ndt) * friction.x;
	velocity.y = (velocity.y + (acceleration.y / mass) * ndt) * friction.y;
	if (velocity.x > maximum_velocity.x) { velocity.x = maximum_velocity.x; }
	if (velocity.x < -maximum_velocity.x) { velocity.x = -maximum_velocity.x; }
	if (velocity.y > maximum_velocity.y) { velocity.y = maximum_velocity.y; }
	if (velocity.y < -maximum_velocity.y) { velocity.y = -maximum_velocity.y; }
	position = position + velocity * ndt;

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

void PhysicsComponent::update() { update_euler(); }

void PhysicsComponent::update_dampen() {
	//acceleration /= svc::tickerLocator.get().tick_rate;
	update_euler();
	acceleration = {0.0f, 0.0f};
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

void PhysicsComponent::hitstun() { dt /= 2.0f; }

void PhysicsComponent::set_constant_friction(sf::Vector2<float> fric) {
	ground_friction = {fric.x, fric.x};
	air_friction = {fric.y, fric.y};
}

} // namespace components