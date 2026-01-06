
#include "fornani/weapon/Projectile.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/weapon/Weapon.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::arms {

Projectile::Projectile(automa::ServiceProvider& svc, std::string_view label, int id, Weapon& weapon, bool enemy)
	: Animatable(svc, "projectile_" + std::string{label}), metadata{.id = id, .label = label}, m_weapon(&weapon),
	  physical{.collider{enemy ? svc.data.enemy_weapon[label]["class_package"]["projectile"]["radius"].as<float>() : svc.data.weapon[label]["class_package"]["projectile"]["radius"].as<float>()}}, m_reflected{48} {

	auto const& in_data = enemy ? svc.data.enemy_weapon[label]["class_package"]["projectile"] : svc.data.weapon[label]["class_package"]["projectile"];

	metadata.type = static_cast<ProjectileType>(in_data["type"].as<int>());
	Animatable::set_dimensions({in_data["dimensions"][0].as<int>(), in_data["dimensions"][1].as<int>()});
	Animatable::set_origin(sf::Vector2f{in_data["dimensions"][0].as<float>(), in_data["dimensions"][1].as<float>()} * 0.5f);
	Animatable::set_parameters(anim::Parameters{0, in_data["animation"]["num_frames"].as<int>(), in_data["animation"]["framerate"].as<int>(), -1});

	metadata.specifications.base_damage = in_data["attributes"]["base_damage"].as<float>();
	metadata.specifications.power = in_data["attributes"]["power"] ? in_data["attributes"]["power"].as<int>() : 1;
	metadata.specifications.speed = in_data["attributes"]["speed"].as<float>();
	metadata.specifications.speed_variance = in_data["attributes"]["speed_variance"].as<float>();
	metadata.specifications.speed += random::random_range_float(-metadata.specifications.speed_variance, metadata.specifications.speed_variance);
	metadata.specifications.variance = in_data["attributes"]["variance"].as<float>();
	metadata.specifications.stun_time = in_data["attributes"]["stun_time"].as<float>();
	metadata.specifications.knockback = in_data["attributes"]["knockback"].as<float>();
	metadata.specifications.acceleration_factor = in_data["attributes"]["acceleration_factor"].as<float>();
	metadata.specifications.dampen_factor = in_data["attributes"]["dampen_factor"].as<float>();
	metadata.specifications.dampen_variance = in_data["attributes"]["dampen_variance"].as<float>();
	metadata.specifications.gravity = in_data["attributes"]["gravity"].as<float>();
	metadata.specifications.elasticty = in_data["attributes"]["elasticity"].as<float>();
	metadata.specifications.spin = in_data["attributes"]["spin"].as<float>();
	metadata.specifications.spin_dampen = in_data["attributes"]["spin_dampen"].as<float>();

	if (in_data["attributes"]["persistent"].as_bool()) { metadata.attributes.set(ProjectileAttributes::persistent); }
	if (in_data["attributes"]["transcendent"].as_bool()) { metadata.attributes.set(ProjectileAttributes::transcendent); }
	if (in_data["attributes"]["constrained"].as_bool()) { metadata.attributes.set(ProjectileAttributes::constrained); }
	if (in_data["attributes"]["omnidirectional"].as_bool()) { metadata.attributes.set(ProjectileAttributes::omnidirectional); }
	if (in_data["attributes"]["boomerang"].as_bool()) { metadata.attributes.set(ProjectileAttributes::boomerang); }
	if (in_data["attributes"]["explode_on_impact"].as_bool()) { metadata.attributes.set(ProjectileAttributes::explode_on_impact); }
	if (in_data["attributes"]["wander"].as_bool()) { metadata.attributes.set(ProjectileAttributes::wander); }
	if (in_data["attributes"]["reflect"].as_bool()) { metadata.attributes.set(ProjectileAttributes::reflect); }
	if (in_data["attributes"]["sprite_flip"].as_bool()) { metadata.attributes.set(ProjectileAttributes::sprite_flip); }
	if (in_data["attributes"]["sticky"].as_bool()) { metadata.attributes.set(ProjectileAttributes::sticky); }

	visual.num_angles = in_data["animation"]["angles"].as<int>();
	visual.effect_type = in_data["visual"]["effect_type"].as<int>();

	audio.hit = static_cast<audio::Projectile>(in_data["audio"]["hit"].as<int>());

	metadata.specifications.lifespan = in_data["attributes"]["lifespan"].as<int>();
	metadata.specifications.lifespan_variance = in_data["attributes"]["lifespan_variance"].as<int>();
	auto var = random::random_range(-metadata.specifications.lifespan_variance, metadata.specifications.lifespan_variance);
	lifetime = util::Cooldown{metadata.specifications.lifespan + var};
	damage_timer = util::Cooldown{in_data["attributes"]["damage_rate"].as<int>()};

	physical.collider.physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
	physical.collider.physics.elasticity = in_data["attributes"]["elasticity"].as<float>();
	physical.collider.physics.gravity = metadata.specifications.gravity;
	if (metadata.specifications.dampen_factor != 0.f) {
		auto var = random::random_range_float(-metadata.specifications.dampen_variance, metadata.specifications.dampen_variance);
		physical.collider.physics.set_global_friction(metadata.specifications.dampen_factor + var);
	}

	// circle
	if (metadata.attributes.test(ProjectileAttributes::circle)) { physical.sensor = components::CircleSensor(physical.collider.get_radius()); }

	variables.state.set(ProjectileState::initialized);
	cooldown.start(40);
	lifetime.start();
	seed(svc);
}

void Projectile::update(automa::ServiceProvider& svc, player::Player& player) {
	tick();
	cooldown.update();
	lifetime.update();
	m_reflected.update();
	damage_timer.update();
	if (variables.state.test(ProjectileState::destruction_initiated) && !metadata.attributes.test(ProjectileAttributes::constrained)) { destroy(true); }

	if (boomerang()) {
		physical.collider.physics.set_global_friction(0.95f);
		physical.steering.seek(physical.collider.physics, player.get_collider().get_center(), 0.001f);
		physical.collider.physics.simple_update();
	} else if (wander()) {
		physical.collider.physics.set_global_friction(0.9f);
		physical.steering.smooth_random_walk(physical.collider.physics, 0.01f);
		physical.collider.physics.simple_update();
	}

	// animation
	static auto rotation = metadata.specifications.spin;
	static auto dampened = metadata.specifications.spin;
	rotation += metadata.specifications.spin;
	dampened *= metadata.specifications.spin_dampen;
	auto rotation_angle = metadata.specifications.spin > 0.f ? physical.collider.physics.velocity.rotatedBy(sf::radians(rotation + dampened)) : physical.collider.physics.velocity;
	if (visual.num_angles > 0 && !sprite_flip() && !is_stuck()) { visual.rotator.handle_rotation(get_sprite(), rotation_angle, visual.num_angles); }
	set_channel(visual.rotator.get_sprite_angle_index());
	if (physical.sensor) { physical.sensor->set_position(physical.collider.get_global_center()); }

	if (lifetime.is_complete()) {
		variables.state.set(ProjectileState::whiffed);
		destroy(false, true);
	}

	if (variables.state.test(arms::ProjectileState::destroyed)) { m_weapon->decrement_projectiles(); }
}

void Projectile::handle_collision(automa::ServiceProvider& svc, world::Map& map) {
	if (!is_stuck()) { physical.collider.update(svc); }
	if (transcendent()) { return; }
	if (reflect()) {
		physical.collider.handle_map_collision(map);
		if (physical.collider.collided() && !m_reflected.running()) {
			svc.soundboard.flags.projectile.set(audio.hit);
			m_reflected.start();
			if (metadata.attributes.test(ProjectileAttributes::explode_on_impact)) {
				map.spawn_explosion(svc, "explosion", physical.collider.get_global_center(), 46.f, 3);
				destroy(false);
			}
		}
		physical.collider.physics.acceleration = {};
		return;
	}
	if (sticky()) {
		physical.collider.handle_map_collision(map);
		if (physical.collider.collided() && !is_stuck()) {
			svc.soundboard.flags.projectile.set(audio.hit);
			variables.state.set(ProjectileState::stuck);
		}
		return;
	}
	if (map.check_cell_collision_circle(physical.collider, false)) {
		if (!destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, "bullet_hit", physical.collider.get_global_center(), {}, effect_type()));
			if (physical.direction.lnr == LNR::neutral) { map.effects.back().rotate(); }
			// auto listener_position = sf::Vector2f{sf::Listener::getPosition().x, sf::Listener::getPosition().z};
			// TODO: use capo engine here
			// svc.soundboard.play(svc, svc.sounds.get_buffer("wall_hit"), 0.1f, 100.f, 0, 10.f, listener_position - physical.bounding_box.get_center());
		}
		destroy(false);
	}
}

void Projectile::on_player_hit(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (metadata.team == arms::Team::nani || is_stuck()) { return; }
	if (player.is_dead()) { return; }
	if (physical.sensor) {
		if (physical.sensor.value().within_bounds(player.hurtbox)) {
			if (metadata.attributes.test(ProjectileAttributes::explode_on_impact)) {
				map.spawn_explosion(svc, "explosion", physical.collider.get_global_center(), 46.f, 3);
				destroy(false);
			} else {
				player.hurt(metadata.specifications.base_damage);
			}
		}
		return;
	}
	if (physical.collider.collides_with(player.hurtbox)) {
		if (metadata.attributes.test(ProjectileAttributes::explode_on_impact)) {
			map.spawn_explosion(svc, "explosion", physical.collider.get_global_center(), 46.f, 3);
		} else {
			player.hurt(metadata.specifications.base_damage);
		}
		destroy(false);
	}
}

void Projectile::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f cam) {

	Animatable::set_position(physical.collider.get_global_center() - cam);
	if (!lifetime.just_started()) { win.draw(*this); }

	// proj bounding box for debug
	if (svc.greyblock_mode()) {
		if (physical.sensor) { physical.sensor.value().render(win, cam); }
		physical.collider.render(win, cam);
		sf::RectangleShape box{{2.f, 2.f}};
		box.setOrigin({1.f, 1.f});
		box.setFillColor(colors::bright_orange);
		box.setPosition(physical.collider.get_global_center() - cam);
		win.draw(box);
	}
}

void Projectile::destroy(bool completely, bool whiffed) {
	if (!whiffed) { variables.state.set(ProjectileState::contact); }
	variables.state.set(ProjectileState::destroyed);
}

void Projectile::seed(automa::ServiceProvider& svc, sf::Vector2f target, float speed_multiplier) {
	float var = random::random_range_float(-metadata.specifications.variance, metadata.specifications.variance);
	metadata.specifications.speed *= speed_multiplier;
	if (omnidirectional()) {
		physical.collider.physics.velocity = util::unit(target) * metadata.specifications.speed;
		return;
	}
	switch (physical.direction.lnr) {
	case LNR::left: physical.collider.physics.velocity = {-metadata.specifications.speed, var}; break;
	case LNR::right: physical.collider.physics.velocity = {metadata.specifications.speed, var}; break;
	case LNR::neutral: break;
	default: NANI_LOG_WARN(m_logger, "Unknown direction was passed. Did you forget to add a case to the switch?"); break;
	}
	switch (physical.direction.und) {
	case UND::up: physical.collider.physics.velocity = {var, -metadata.specifications.speed}; break;
	case UND::down: physical.collider.physics.velocity = {var, metadata.specifications.speed}; break;
	case UND::neutral: break;
	default: NANI_LOG_WARN(m_logger, "Unknown direction was passed. Did you forget to add a case to the switch?"); break;
	}
	if (sprite_flip()) {
		auto scale = physical.direction.left_or_right() ? sf::Vector2f{1.f, -1.f} : sf::Vector2f{-1.f, 1.f};
		if (random::percent_chance(50)) { set_scale(scale * constants::f_scale_factor); }
	}
}

void Projectile::set_position(sf::Vector2f pos) {
	physical.collider.physics.position = pos;
	variables.fired_point = pos;
}

void Projectile::set_team(Team to_team) { metadata.team = to_team; }

void Projectile::set_firing_direction(Direction to_direction) { physical.direction = to_direction; }

void Projectile::poof() { variables.state.set(arms::ProjectileState::poof); }

void Projectile::damage_over_time() {
	if (damage_timer.is_complete()) { damage_timer.start(); }
}

void Projectile::bounce_off_surface(sf::Vector2i direction) {
	return;
	if (std::abs(direction.x) > 0) {
		physical.collider.physics.acceleration.x *= -1.f * metadata.specifications.elasticty;
		physical.collider.physics.velocity.x *= -1.f * metadata.specifications.elasticty;
	}
	if (std::abs(direction.y) > 0) {
		physical.collider.physics.acceleration.y *= -1.f * metadata.specifications.elasticty;
		physical.collider.physics.velocity.y *= -1.f * metadata.specifications.elasticty;
	}
}

} // namespace fornani::arms
