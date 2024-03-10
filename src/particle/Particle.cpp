
#include <algorithm>
#include "Particle.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Particle::Particle(components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric, float sz, dir::Direction dir_) : physics(p), init_force(f), force_variance(v), angle_range(a), size(sz), direction(dir_) {
	lifespan = svc::randomLocator.get().random_range(default_lifespan, 100);
	physics.set_constant_friction(fric);
	float randx{};
	float randy{};
	switch (direction.lr) {
	case dir::LR::left:
		randx = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		randy = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	case dir::LR::right:
		randx = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance);
		randy = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		break;
	}
	switch (direction.und) {
	case dir::UND::up:
		randx = svc::randomLocator.get().random_range_float(-angle_range, angle_range);
		randy = svc::randomLocator.get().random_range_float(init_force - force_variance, init_force + force_variance) * -1;
		break;
	case dir::UND::down:
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

void Particle::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	dot.setFillColor(color);
	dot.setSize({size, size});
	dot.setPosition(physics.position.x - cam.x, physics.position.y - cam.y);
	win.draw(dot);
	svc::counterLocator.get().at(svc::draw_calls)++;
}

void Particle::oscillate_between_colors(sf::Color dark, sf::Color bright) {

	int low_r = std::min(bright.r, dark.r);
	int low_g = std::min(bright.g, dark.g);
	int low_b = std::min(bright.b, dark.b);
	int high_r = std::max(bright.r, dark.r);
	int high_g = std::max(bright.g, dark.g);
	int high_b = std::max(bright.b, dark.b);

	bool red = high_r > high_b && high_r > high_g;
	bool green = high_g > high_r && high_g > high_b;
	bool blue = high_b > high_r && high_b > high_g;

	int rand_r = svc::randomLocator.get().random_range(low_r, high_r);
	int rand_g = svc::randomLocator.get().random_range(low_g, high_g);
	int rand_b = svc::randomLocator.get().random_range(low_b, high_b);

	auto r = red ? bright.r : std::clamp(rand_r, low_r, high_r);
	auto g = green ? bright.g : std::clamp(rand_g, low_g, high_g);
	auto b = blue ? bright.b : std::clamp(rand_b, low_b, high_b);

	color = sf::Color(r, g, b);

}

} // namespace vfx