
#include <algorithm>
#include "Particle.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Particle::Particle(services::ServiceLocator& svc, components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric, float sz, dir::Direction dir_)
	: physics(p), init_force(f), force_variance(v), angle_range(a), size(sz), direction(dir_) {
	lifespan = svc.randomLocator.get().random_range(default_lifespan, 100);
	physics.set_constant_friction(fric);
	float randx{};
	float randy{};
	switch (direction.lr) {
	case dir::LR::left:
		randx = svc.randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		randy = svc.randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	case dir::LR::right:
		randx = svc.randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance);
		randy = svc.randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	}
	switch (direction.und) {
	case dir::UND::up:
		randx = svc.randomLocator.get().random_range_float(-angle_range, angle_range);
		randy = svc.randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		break;
	case dir::UND::down:
		randx = svc.randomLocator.get().random_range_float(-angle_range, angle_range);
		randy = svc.randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance);
		break;
	}
	physics.velocity.x = randx * init_force;
	physics.velocity.y = randy * init_force;
}

void Particle::update(float initial_force, float grav, float grav_variance, services::ServiceLocator& svc) {
	float var = svc.randomLocator.get().random_range_float(-grav_variance, grav_variance);
	physics.acceleration.y = grav + var;
	physics.update_dampen();
	bounding_box.dimensions = sf::Vector2<float>(default_dim, default_dim);
	bounding_box.set_position(physics.position);
	--lifespan;
}

void Particle::render(sf::RenderWindow& win, sf::Vector2<float> cam, services::ServiceLocator& svc) {
	dot.setFillColor(color);
	dot.setSize({size, size});
	dot.setPosition(physics.position.x - cam.x, physics.position.y - cam.y);
	win.draw(dot);
	
}

void Particle::oscillate_between_colors(sf::Color dark, sf::Color bright) { color = bright; }

} // namespace vfx