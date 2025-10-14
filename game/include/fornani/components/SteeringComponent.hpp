
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>

namespace fornani::components {

struct SteeringComponent {
	void seek(sf::Vector2f const target, float force = 0.01f) {
		steering.seek(physics, target, force);
		physics.simple_update();
	}
	SteeringBehavior steering{};
	PhysicsComponent physics{};
};

} // namespace fornani::components
