
#include "fornani/weapon/Projectile.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/weapon/Weapon.hpp"

namespace fornani::arms {

Projectile::Projectile(automa::ServiceProvider& svc, std::string_view label, int id, Weapon& weapon, bool enemy) : metadata{.id = id, .label = label}, m_weapon(&weapon), visual{.sprite{svc.assets.projectile_textures.at(label)}} {

	auto const& in_data = enemy ? svc.data.enemy_weapon["weapons"][id]["class_package"]["projectile"] : svc.data.weapon["weapons"][id]["class_package"]["projectile"];

	metadata.type = static_cast<ProjectileType>(in_data["type"].as<int>());
	physical.bounding_box = shape::Shape({in_data["dimensions"][0].as<float>(), in_data["dimensions"][1].as<float>()});
	visual.sprite.set_dimensions({in_data["dimensions"][0].as<int>(), in_data["dimensions"][1].as<int>()});

	metadata.specifications.base_damage = in_data["attributes"]["base_damage"].as<float>();
	metadata.specifications.power = in_data["attributes"]["power"] ? in_data["attributes"]["power"].as<int>() : 1;
	metadata.specifications.speed = in_data["attributes"]["speed"].as<float>();
	metadata.specifications.speed_variance = in_data["attributes"]["speed_variance"].as<float>();
	metadata.specifications.speed += util::random::random_range_float(-metadata.specifications.speed_variance, metadata.specifications.speed_variance);
	metadata.specifications.variance = in_data["attributes"]["variance"].as<float>();
	metadata.specifications.stun_time = in_data["attributes"]["stun_time"].as<float>();
	metadata.specifications.knockback = in_data["attributes"]["knockback"].as<float>();
	metadata.specifications.acceleration_factor = in_data["attributes"]["acceleration_factor"].as<float>();
	metadata.specifications.dampen_factor = in_data["attributes"]["dampen_factor"].as<float>();
	metadata.specifications.dampen_variance = in_data["attributes"]["dampen_variance"].as<float>();
	metadata.specifications.gravity = in_data["attributes"]["gravity"].as<float>();
	metadata.specifications.elasticty = in_data["attributes"]["elasticity"].as<float>();

	if (static_cast<bool>(in_data["attributes"]["persistent"].as_bool())) { metadata.attributes.set(ProjectileAttributes::persistent); }
	if (static_cast<bool>(in_data["attributes"]["transcendent"].as_bool())) { metadata.attributes.set(ProjectileAttributes::transcendent); }
	if (static_cast<bool>(in_data["attributes"]["constrained"].as_bool())) { metadata.attributes.set(ProjectileAttributes::constrained); }
	if (static_cast<bool>(in_data["attributes"]["omnidirectional"].as_bool())) { metadata.attributes.set(ProjectileAttributes::omnidirectional); }
	if (static_cast<bool>(in_data["attributes"]["boomerang"].as_bool())) { metadata.attributes.set(ProjectileAttributes::boomerang); }
	if (static_cast<bool>(in_data["attributes"]["wander"].as_bool())) { metadata.attributes.set(ProjectileAttributes::wander); }
	if (static_cast<bool>(in_data["attributes"]["reflect"].as_bool())) { metadata.attributes.set(ProjectileAttributes::reflect); }
	if (static_cast<bool>(in_data["attributes"]["sprite_flip"].as_bool())) { metadata.attributes.set(ProjectileAttributes::sprite_flip); }

	visual.sprite.push_params("anim", {0, in_data["animation"]["num_frames"].as<int>(), in_data["animation"]["framerate"].as<int>(), -1});
	visual.sprite.set_params("anim");
	visual.num_angles = in_data["animation"]["angles"].as<int>();
	visual.effect_type = in_data["visual"]["effect_type"].as<int>();
	visual.render_type = visual.sprite.size() > 1 ? RenderType::multi_sprite : RenderType::single_sprite;

	metadata.specifications.lifespan = in_data["attributes"]["lifespan"].as<int>();
	metadata.specifications.lifespan_variance = in_data["attributes"]["lifespan_variance"].as<int>();
	auto var = util::random::random_range(-metadata.specifications.lifespan_variance, metadata.specifications.lifespan_variance);
	lifetime = util::Cooldown{metadata.specifications.lifespan + var};
	damage_timer = util::Cooldown{in_data["attributes"]["damage_rate"].as<int>()};

	physical.physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
	physical.physics.velocity.x = metadata.specifications.speed;
	if (metadata.specifications.dampen_factor != 0.f) {
		auto var = util::random::random_range_float(-metadata.specifications.dampen_variance, metadata.specifications.dampen_variance);
		physical.physics.set_global_friction(metadata.specifications.dampen_factor + var);
	}

	// circle
	if (metadata.attributes.test(ProjectileAttributes::circle)) { physical.sensor = components::CircleSensor(physical.bounding_box.get_dimensions().x * 0.5f); }
	visual.sprite.set_origin(physical.bounding_box.get_dimensions() * 0.5f);

	physical.max_dimensions = physical.bounding_box.get_dimensions();
	physical.physics.gravity = metadata.specifications.gravity;

	variables.state.set(ProjectileState::initialized);
	cooldown.start(40);
	lifetime.start();
	seed(svc);
}

void Projectile::update(automa::ServiceProvider& svc, player::Player& player) {

	cooldown.update();
	lifetime.update();
	damage_timer.update();
	if (variables.state.test(ProjectileState::destruction_initiated) && !metadata.attributes.test(ProjectileAttributes::constrained)) { destroy(true); }

	if (boomerang()) {
		physical.physics.set_global_friction(0.95f);
		physical.steering.seek(physical.physics, player.collider.get_center(), 0.001f);
		physical.physics.simple_update();
	} else if (wander()) {
		physical.physics.set_global_friction(0.9f);
		physical.steering.smooth_random_walk(physical.physics, 0.01f);
		physical.physics.simple_update();
	} else {
		physical.physics.update_euler(svc);
	}

	// animation
	if (visual.num_angles > 0 && !sprite_flip()) { visual.sprite.handle_rotation(physical.physics.velocity, visual.num_angles); }
	visual.sprite.update(physical.bounding_box.get_position() + physical.bounding_box.get_dimensions() * 0.5f, 0, visual.sprite.get_sprite_angle_index(), true);
	if (physical.sensor) { visual.sprite.update(physical.bounding_box.get_position() + physical.bounding_box.get_dimensions() * 0.5f, 0, visual.sprite.get_sprite_angle_index(), true); }

	if (physical.direction.lr == dir::LR::left) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x, physical.physics.position.y - physical.bounding_box.get_dimensions().y / 2});
	} else if (physical.direction.lr == dir::LR::right) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.get_dimensions().x, physical.physics.position.y - physical.bounding_box.get_dimensions().y / 2});
	} else if (physical.direction.und == dir::UND::up) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.get_dimensions().x / 2, physical.physics.position.y});
	} else if (physical.direction.und == dir::UND::down) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.get_dimensions().x / 2, physical.physics.position.y - physical.bounding_box.get_dimensions().y});
	}

	if (physical.sensor) { physical.sensor.value().set_position(physical.bounding_box.get_position()); }

	if (lifetime.is_complete()) {
		variables.state.set(ProjectileState::whiffed);
		destroy(false, true);
	}

	if (variables.state.test(arms::ProjectileState::destroyed)) { m_weapon->decrement_projectiles(); }
}

void Projectile::handle_collision(automa::ServiceProvider& svc, world::Map& map) {
	if (transcendent()) { return; }
	physical.collider.update(svc);
	physical.collider.set_position(physical.physics.position);
	if (map.check_cell_collision_circle(physical.collider, false)) {
		if (!destruction_initiated()) {
			map.effects.push_back(entity::Effect(svc, variables.destruction_point + physical.physics.position, {}, effect_type(), 2));
			if (physical.direction.lr == dir::LR::neutral) { map.effects.back().rotate(); }
			auto listener_position = sf::Vector2<float>{sf::Listener::getPosition().x, sf::Listener::getPosition().z};
			svc.soundboard.play(svc, svc.assets.b_soft_tap, 0.1f, 100.f, 0, 10.f, listener_position - physical.bounding_box.get_center());
		}
		destroy(false);
	}
}

void Projectile::on_player_hit(player::Player& player) {
	if (metadata.team == arms::Team::nani) { return; }
	if (physical.sensor) {
		if (physical.sensor.value().within_bounds(player.hurtbox)) { player.hurt(metadata.specifications.base_damage); }
		return;
	}
	if (physical.bounding_box.overlaps(player.hurtbox)) {
		player.hurt(metadata.specifications.base_damage);
		destroy(false);
	}
}

void Projectile::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam) {

	if (!lifetime.just_started()) { visual.sprite.render(svc, win, cam); }

	// proj bounding box for debug
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		box.setSize(physical.bounding_box.get_dimensions());
		if (variables.state.test(ProjectileState::destruction_initiated)) {
			box.setFillColor(sf::Color{255, 255, 60, 160});
		} else {
			box.setFillColor(sf::Color{255, 255, 255, 160});
		}
		box.setPosition(physical.bounding_box.get_position() - cam);
		win.draw(box);
		if (physical.sensor) { physical.sensor.value().render(win, cam); }
		physical.collider.render(win, cam);
	}
}

void Projectile::destroy(bool completely, bool whiffed) {

	if (!whiffed) { variables.state.set(ProjectileState::contact); }

	if (completely) {
		variables.state.set(ProjectileState::destroyed);
		return;
	}

	if (!variables.state.test(ProjectileState::destruction_initiated)) {
		if (physical.direction.lr == dir::LR::left || physical.direction.und == dir::UND::up) {
			variables.destruction_point = physical.bounding_box.get_position();
		} else {
			variables.destruction_point = physical.bounding_box.get_position() + physical.bounding_box.get_dimensions();
		}
		variables.state.set(ProjectileState::destruction_initiated);
	}

	metadata.specifications.base_damage = 0;
}

void Projectile::seed(automa::ServiceProvider& svc, sf::Vector2<float> target) {
	float var = util::random::random_range_float(-metadata.specifications.variance, metadata.specifications.variance);
	if (omnidirectional()) {
		physical.physics.velocity = util::unit(target) * metadata.specifications.speed;
		return;
	}
	switch (physical.direction.lr) {
	case dir::LR::left: physical.physics.velocity = {-metadata.specifications.speed, var}; break;
	case dir::LR::right: physical.physics.velocity = {metadata.specifications.speed, var}; break;
	case dir::LR::neutral: break;
	default: NANI_LOG_WARN(m_logger, "Unknown direction was passed. Did you forget to add a case to the switch?"); break;
	}
	switch (physical.direction.und) {
	case dir::UND::up: physical.physics.velocity = {var, -metadata.specifications.speed}; break;
	case dir::UND::down: physical.physics.velocity = {var, metadata.specifications.speed}; break;
	case dir::UND::neutral: break;
	default: NANI_LOG_WARN(m_logger, "Unknown direction was passed. Did you forget to add a case to the switch?"); break;
	}
	if (sprite_flip()) {
		auto scale = physical.direction.left_or_right() ? sf::Vector2<float>{1.f, -1.f} : sf::Vector2<float>{-1.f, 1.f};
		if (util::random::percent_chance(50)) { visual.sprite.set_scale(scale); }
	}
}

void Projectile::set_position(sf::Vector2<float> pos) {
	physical.physics.position = pos;
	physical.bounding_box.set_position(pos);
	variables.fired_point = pos;
	visual.sprite.set_position(pos);
}

void Projectile::set_team(Team to_team) { metadata.team = to_team; }

void Projectile::set_firing_direction(dir::Direction to_direction) { physical.direction = to_direction; }

void Projectile::poof() { variables.state.set(arms::ProjectileState::poof); }

void Projectile::damage_over_time() {
	if (damage_timer.is_complete()) { damage_timer.start(); }
}

} // namespace fornani::arms
