
#include "Emitter.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Emitter::Emitter(ElementBehavior b, EmitterStats s, sf::Color c) : behavior(b), stats(s), color(c) {
	int var = svc::randomLocator.get().random_range(-stats.lifespan_variance, stats.lifespan_variance);
	stats.lifespan += var;
}
Emitter::~Emitter() { particles.clear(); }

void Emitter::update() { // this will tick every element and the generator itself
	physics.update();
	if (stats.lifespan > 0) { // make a particle at a certain rate
		for (int i = 0; i < behavior.rate; ++i) {
			particles.push_back(Particle(physics, behavior.expulsion_force, behavior.expulsion_variance, behavior.cone, {behavior.x_friction, behavior.y_friction}, stats.part_size));
			particles.back().physics.dir = physics.dir;
			int var = svc::randomLocator.get().random_range(-stats.particle_lifespan_variance, stats.particle_lifespan_variance);
			particles.back().lifespan = stats.particle_lifespan + var;
		}
	}

	for (auto& particle : particles) { particle.update(behavior.expulsion_force, behavior.grav, behavior.grav_variance); }

	std::erase_if(particles, [](auto const& p) { return p.lifespan < 0; });

	--stats.lifespan;
}

bool Emitter::empty() { return particles.empty(); }

void Emitter::set_position(float x, float y) {
	physics.position.x = x;
	physics.position.y = y;
	for (auto& particle : particles) { particle.physics.position = {x, y}; }
}
void Emitter::set_velocity(float x, float y) {
	physics.velocity.x = x;
	physics.velocity.y = y;
}

void Emitter::apply_force(sf::Vector2<float> force) {
	physics.apply_force(force);
	for (auto& particle : particles) { particle.physics.apply_force(force); }
}
void Emitter::apply_force_at_angle(float force, float angle) {
	physics.apply_force_at_angle(force, angle);
	for (auto& particle : particles) { particle.physics.apply_force_at_angle(force, angle); }
}

components::PhysicsComponent& Emitter::get_physics() { return physics; }
ElementBehavior& Emitter::get_behavior() { return behavior; }

void Emitter::set_rate(float r) { behavior.rate = r; }
void Emitter::set_expulsion_force(float f) { behavior.expulsion_force = f; }
void Emitter::set_friction(float f) { physics.set_constant_friction({f, f}); }
void Emitter::set_lifespan(int l) { stats.lifespan = l; }
void Emitter::set_direction(components::DIRECTION d) { physics.dir = d; }

std::vector<Particle>& const Emitter::get_particles() { return particles; }

} // namespace vfx
