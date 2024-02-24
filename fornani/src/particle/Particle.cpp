
#include "Particle.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Particle::Particle(components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric, float sz) : physics(p), init_force(f), force_variance(v), angle_range(a), size(sz) {
	lifespan = svc::randomLocator.get().random_range(default_lifespan, 100);
	physics.set_constant_friction(fric);
	float randx{};
	float randy{};
	switch (physics.dir) {
	case components::DIRECTION::LEFT:
		randx = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		randy = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	case components::DIRECTION::RIGHT:
		randx = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance);
		randy = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	case components::DIRECTION::UP:
		randx = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		randy = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		break;
	case components::DIRECTION::DOWN:
		randx = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		randy = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance);
		break;
	}
	physics.velocity.x = randx * init_force;
	physics.velocity.y = randy * init_force;
}
void Particle::update(float initial_force, float grav, float grav_variance) {
	float var = svc::randomLocator.get().random_range_float(-grav_variance, grav_variance);
	physics.acceleration.y = grav + var;
	physics.update_dampen();
	bounding_box.dimensions = sf::Vector2<float>(default_dim, default_dim);
	bounding_box.set_position(physics.position);
	--lifespan;
}

} // namespace vfx