
#include <fornani/core/Debug.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/particle/Particle.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <numbers>

namespace fornani::vfx {

Particle::Particle(automa::ServiceProvider& svc, sf::Vector2f pos, sf::Vector2f dim, std::string_view type, sf::Color color, Direction direction, int channel) : position(pos), dimensions(dim) {
	box.setFillColor(color);
	box.setSize(dimensions);
	box.setOrigin(dimensions * 0.5f);

	m_physics.emplace();

	auto const& in_data = svc.data.particle[type];
	auto expulsion = in_data["expulsion"].as<float>();
	auto expulsion_variance = in_data["expulsion_variance"].as<float>();
	auto angle_range = in_data["cone"].as<float>();
	auto ev = in_data["elasticity_variance"].as<float>();
	if (in_data["animation"].is_object()) {
		m_animatable = Animatable{svc, "particle_" + std::string{type}, sf::Vector2i{dim}};
		// for animated particles
		auto const& in_animation = in_data["animation"];
		auto lookup = in_animation["lookup"].as<int>();
		auto duration = in_animation["duration"].as<int>();
		auto framerate = in_animation["framerate"].as<int>();
		auto loop = in_animation["loop"].as<int>();
		m_animatable->set_parameters({lookup, duration, framerate, loop});
	}

	expulsion += random::random_range_float(-expulsion_variance, expulsion_variance);
	auto angle = random::random_range_float(-angle_range, angle_range);
	auto f_pi = static_cast<float>(std::numbers::pi);
	if (direction.lnr == LNR::left) { angle += f_pi; }
	if (direction.und == UND::up) { angle += f_pi * 1.5f; }
	if (direction.und == UND::down) { angle += f_pi * 0.5f; }

	m_physics->set_global_friction(in_data["friction"].as<float>());
	m_physics->gravity = in_data["gravity"].as<float>();

	auto lifespan_time = in_data["lifespan"].as<int>();
	auto lifespan_variance = in_data["lifespan_variance"].as<int>();
	int rand_diff = random::random_range(-lifespan_variance, lifespan_variance);
	lifespan.start(lifespan_time + rand_diff);

	if (m_animatable) {
		m_animatable->center();
		m_animatable->set_channel(channel);
		if (random::coin_flip()) { m_flip.set(SpriteFlip::horizontal); }
	}

	if (in_data["fader"].as_bool()) { m_fader = util::Fader(svc, lifespan.get(), in_data["color"].as_string()); }
	if (m_fader) { m_fader->get_sprite().setScale(dim); }

	m_physics->apply_force_at_angle(expulsion, angle);
	m_physics->position = position;
}

Particle::Particle(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f pos, sf::Vector2f dim, std::string_view type, sf::Color color, Direction direction, int channel) : Particle{svc, pos, dim, type, color, direction, channel} {
	m_physics.reset();
	m_collider.emplace(map, dim.x);

	auto const& in_data = svc.data.particle[type];
	auto expulsion = in_data["expulsion"].as<float>();
	auto expulsion_variance = in_data["expulsion_variance"].as<float>();
	auto angle_range = in_data["cone"].as<float>();
	auto ev = in_data["elasticity_variance"].as<float>();
	auto angle = random::random_range_float(-angle_range, angle_range);
	auto f_pi = static_cast<float>(std::numbers::pi);
	if (direction.lnr == LNR::left) { angle += f_pi; }
	if (direction.und == UND::up) { angle += f_pi * 1.5f; }
	if (direction.und == UND::down) { angle += f_pi * 0.5f; }
	expulsion += random::random_range_float(-expulsion_variance, expulsion_variance);

	if (!in_data["colliding"].as_bool()) { m_collider->get_circle()->set_attribute(shape::ColliderAttributes::no_collision); }
	if (in_data["physics"]) { m_collider->get_circle()->load_properties(in_data["physics"]); }

	m_collider->get_circle()->physics.elasticity = in_data["elasticity"].as<float>() + random::random_range_float(-ev, ev);
	m_collider->get_circle()->physics.set_global_friction(in_data["friction"].as<float>());
	m_collider->get_circle()->physics.gravity = in_data["gravity"].as<float>();

	m_collider->get_circle()->set_trait(shape::ColliderTrait::particle);
	m_collider->get_circle()->set_exclusion_target(shape::ColliderTrait::player);
	m_collider->get_circle()->set_exclusion_target(shape::ColliderTrait::circle);
	m_collider->get_circle()->set_exclusion_target(shape::ColliderTrait::npc);
	m_collider->get_circle()->set_exclusion_target(shape::ColliderTrait::enemy);
	m_collider->get_circle()->set_exclusion_target(shape::ColliderTrait::particle);

	m_collider->get_circle()->physics.apply_force_at_angle(expulsion, angle);
	m_collider->get_circle()->physics.position = pos;
}

void Particle::update(automa::ServiceProvider& svc, world::Map& map) {
	if (m_animatable) { m_animatable->tick(); }
	if (m_fader) { m_fader->update(); }
	if (m_physics) {
		m_physics->update(svc);
		m_physics->acceleration = {};
	}
	lifespan.update();
}

void Particle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		if (m_collider) { m_collider->get_circle()->render(win, cam); }
		render(win, cam);
	}
}

void Particle::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto render_position = m_collider ? m_collider->get_circle()->physics.position - cam : m_physics ? m_physics->position - cam : sf::Vector2f{};
	if (m_animatable) {
		m_animatable->set_position(render_position);
		win.draw(*m_animatable);
	} else if (m_fader) {
		m_fader->get_sprite().setPosition(render_position);
		win.draw(m_fader->get_sprite());
	} else {
		box.setPosition(render_position);
		win.draw(box);
	}
	++debug::draw_calls;
}

void Particle::submit(Renderer& renderer) {
	auto render_position = m_collider ? m_collider->get_circle()->physics.position : m_physics ? m_physics->position : sf::Vector2f{};
	if (m_animatable || m_fader) {
		auto const pos = m_animatable ? render_position - m_animatable->get_f_dimensions() : render_position;
		auto const& sprite_ref = m_animatable ? m_animatable->get_sprite() : m_fader->get_sprite();
		auto const& frame = sprite_ref.getTextureRect();
		sf::FloatRect dest{pos, sf::Vector2f{static_cast<float>(frame.size.x), static_cast<float>(frame.size.y)}};
		auto scale = m_fader ? dimensions.x : constants::f_scale_factor;
		renderer.submit(sprite_ref.getTexture(), dest, frame, scale, sf::Color::White, m_flip);
	} else {
		auto const& frame = box.getTextureRect();
		sf::FloatRect dest{render_position, sf::Vector2f{static_cast<float>(frame.size.x), static_cast<float>(frame.size.y)}};
		renderer.submit(dest, frame, dimensions.x);
	}
}

} // namespace fornani::vfx
