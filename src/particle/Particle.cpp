
#include "Particle.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../service/ServiceProvider.hpp"

namespace vfx {

Particle::Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, dir::Direction direction) : position(pos), dimensions(dim) {
	collider = shape::Collider(dimensions);
	collider.sync_components();
	box.setFillColor(color);
	box.setSize(dimensions);

	auto const& in_data = svc.data.particle[type];
	auto expulsion = in_data["expulsion"].as<float>();
	auto angle = in_data["cone"].as<float>();
	collider.physics.set_global_friction(in_data["friction"].as<float>());
	collider.stats.GRAV = in_data["gravity"].as<float>();

	expulsion = direction.lr == dir::LR::left ? -expulsion : expulsion;

	collider.physics.apply_force_at_angle(expulsion, angle);
	collider.physics.position = position;


	int rand_diff = svc::randomLocator.get().random_range(0, 50);
	lifespan.start(200 + rand_diff);
}

void Particle::update(world::Map& map) {

	collider.update();
	collider.detect_map_collision(map);
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};

	lifespan.update();
}

void Particle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { 
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_mode)) {
		collider.render(win, cam);
	} else {
		box.setPosition(collider.physics.position - cam);
		win.draw(box);
	}
}


} // namespace vfx