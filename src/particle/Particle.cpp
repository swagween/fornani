
#include "Particle.hpp"
#include "../service/ServiceProvider.hpp"
#include <numbers>

namespace vfx {

Particle::Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, dir::Direction direction) : position(pos), dimensions(dim), sprite_dimensions(dim) {
	collider = shape::Collider(dimensions);
	collider.sync_components();
	box.setFillColor(color);
	box.setSize(dimensions);

	auto const& in_data = svc.data.particle[type];
	auto expulsion = in_data["expulsion"].as<float>();
	auto expulsion_variance = in_data["expulsion_variance"].as<float>();
	auto angle_range = in_data["cone"].as<float>();
	collider.physics.elasticity = in_data["elasticity"].as<float>();
	collider.physics.set_global_friction(in_data["friction"].as<float>());
	collider.stats.GRAV = in_data["gravity"].as<float>();
	if ((dj::Boolean)in_data["animated"].as_bool()) { flags.set(ParticleType::animated); }
	if ((dj::Boolean)in_data["colliding"].as_bool()) { flags.set(ParticleType::colliding); }

	auto angle = svc.random.random_range_float(-angle_range, angle_range);
	if (direction.lr == dir::LR::left) { angle += std::numbers::pi; }
	if (direction.und == dir::UND::up) { angle += std::numbers::pi * 1.5; }
	if (direction.und == dir::UND::down) { angle += std::numbers::pi * 0.5; }

	expulsion += svc.random.random_range(-expulsion_variance, expulsion_variance);

	collider.physics.apply_force_at_angle(expulsion, angle);
	collider.physics.position = position;

	auto lifespan_time = in_data["lifespan"].as<int>();
	auto lifespan_variance = in_data["lifespan_variance"].as<int>();
	int rand_diff = svc.random.random_range(-lifespan_variance, lifespan_variance);
	lifespan.start(lifespan_time + rand_diff);

	//for animated particles
	sprite.setOrigin(dimensions * 0.5f);
	if (svc.assets.particle_textures.contains(type)) { sprite.setTexture(svc.assets.particle_textures.at(type)); }
	auto const& in_animation = in_data["animation"];
	auto lookup = in_animation["lookup"].as<int>();
	auto duration = in_animation["duration"].as<int>();
	auto framerate = in_animation["framerate"].as<int>();
	auto loop = in_animation["loop"].as<int>();
	animation.set_params({lookup, duration, framerate, loop});
}

void Particle::update(automa::ServiceProvider& svc, world::Map& map) {

	collider.update(svc);
	if (flags.test(ParticleType::colliding)) { collider.detect_map_collision(map); }
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	animation.update();
	lifespan.update();
}

void Particle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { 
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		if (flags.test(ParticleType::animated)) {
			sprite.setTextureRect(sf::IntRect{{0, animation.get_frame() * sprite_dimensions.y}, sprite_dimensions});
			sprite.setPosition(collider.physics.position - cam);
			if (!animation.complete()) { win.draw(sprite); }
		} else {
			box.setPosition(collider.physics.position - cam);
			win.draw(box);
		}
	}
}


} // namespace vfx