
#include "fornani/particle/Particle.hpp"

#include <tracy/Tracy.hpp>

#include <numbers>
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Particle::Particle(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Vector2<float> dim, std::string_view type, sf::Color color, dir::Direction direction)
	: position(pos), dimensions(dim), sprite_dimensions(dim), collider(dim.x), sprite{svc.assets.get_texture("particle_" + std::string{type})} {
	box.setFillColor(color);
	box.setSize(dimensions);
	box.setOrigin(dimensions * 0.5f);

	auto const& in_data = svc.data.particle[type];
	auto expulsion = in_data["expulsion"].as<float>();
	auto expulsion_variance = in_data["expulsion_variance"].as<float>();
	auto angle_range = in_data["cone"].as<float>();
	collider.physics.elasticity = in_data["elasticity"].as<float>();
	collider.physics.set_global_friction(in_data["friction"].as<float>());
	collider.physics.gravity = in_data["gravity"].as<float>();
	if ((dj::Boolean)in_data["animated"].as_bool()) { flags.set(ParticleType::animated); }
	if ((dj::Boolean)in_data["colliding"].as_bool()) { flags.set(ParticleType::colliding); }

	auto angle = util::Random::random_range_float(-angle_range, angle_range);
	auto f_pi = static_cast<float>(std::numbers::pi);
	if (direction.lr == dir::LR::left) { angle += f_pi; }
	if (direction.und == dir::UND::up) { angle += f_pi * 1.5f; }
	if (direction.und == dir::UND::down) { angle += f_pi * 0.5f; }

	expulsion += util::Random::random_range_float(-expulsion_variance, expulsion_variance);

	collider.physics.apply_force_at_angle(expulsion, angle);
	collider.physics.position = position;

	auto lifespan_time = in_data["lifespan"].as<int>();
	auto lifespan_variance = in_data["lifespan_variance"].as<int>();
	int rand_diff = util::Random::random_range(-lifespan_variance, lifespan_variance);
	lifespan.start(lifespan_time + rand_diff);

	// for animated particles
	auto const& in_animation = in_data["animation"];
	auto lookup = in_animation["lookup"].as<int>();
	auto duration = in_animation["duration"].as<int>();
	auto framerate = in_animation["framerate"].as<int>();
	auto loop = in_animation["loop"].as<int>();
	animation.set_params({lookup, duration, framerate, loop});
	if (util::Random::percent_chance(50)) { sprite.scale({-1.f, 1.f}); }
	sprite.setOrigin(dimensions * 0.5f);

	if (in_data["fader"].as_bool()) { fader = util::Fader(svc, lifespan.get_cooldown(), in_data["color"].as_string()); }
	if (fader) { fader.value().get_sprite().setScale(dim); }
}

void Particle::update(automa::ServiceProvider& svc, world::Map& map) {
	collider.update(svc);
	if (flags.test(ParticleType::colliding)) { collider.handle_map_collision(map); }
	collider.physics.acceleration = {};
	animation.update();
	lifespan.update();
	if (fader) { fader.value().update(); }
}

void Particle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	ZoneScopedN("Particle::render");
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		if (flags.test(ParticleType::animated)) {
			sprite.setTextureRect(sf::IntRect{{0, animation.get_frame() * sprite_dimensions.y}, sprite_dimensions});
			sprite.setPosition(collider.physics.position - cam);
			win.draw(sprite);
		} else if (fader) {
			fader.value().get_sprite().setPosition(collider.physics.position - cam);
			win.draw(fader.value().get_sprite());
		} else {
			box.setPosition(collider.physics.position - cam);
			win.draw(box);
		}
	}
}

} // namespace fornani::vfx
