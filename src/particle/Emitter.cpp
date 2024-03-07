
#include "Emitter.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Emitter::Emitter(ElementBehavior behavior, EmitterStats stats, sf::Color bright_color, sf::Color dark_color) : behavior(behavior), stats(stats), bright(bright_color), dark(dark_color) {
	int var = svc::randomLocator.get().random_range(-stats.lifespan_variance, stats.lifespan_variance);
	stats.lifespan += var;
}
Emitter::~Emitter() { particles.clear(); }

void Emitter::update() { // this will tick every element and the generator itself
	physics.update();
	if (stats.lifespan > 0) { // make a particle at a certain rate
		for (int i = 0; i < behavior.rate; ++i) {
			particles.push_back(Particle(physics, behavior.expulsion_force, behavior.expulsion_variance, behavior.cone, {behavior.x_friction, behavior.y_friction}, stats.part_size, direction));
			int var = svc::randomLocator.get().random_range(-stats.particle_lifespan_variance, stats.particle_lifespan_variance);
			particles.back().lifespan = stats.particle_lifespan + var;
		}
	}

	for (auto& particle : particles) { particle.update(behavior.expulsion_force, behavior.grav, behavior.grav_variance); }

	std::erase_if(particles, [](auto const& p) { return p.lifespan < 0; });

	--stats.lifespan;
}

void Emitter::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		for (auto& particle : particles) {
			if ((int)particle.lifespan % 8 == 0) { particle.oscillate_between_colors(dark, bright); }
			particle.render(win, cam);
		}
	}
}

bool Emitter::empty() const { return particles.empty(); }

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
void Emitter::set_direction(dir::Direction d) { direction = d; }

std::vector<Particle>& const Emitter::get_particles() { return particles; }

} // namespace vfx
