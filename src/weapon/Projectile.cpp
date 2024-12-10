
#include "Projectile.hpp"
#include "Weapon.hpp"
#include "class/BulletPackage.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Math.hpp"

namespace arms {

Projectile::Projectile(automa::ServiceProvider& svc, std::string_view label, int id, Weapon& weapon) : metadata{.id = id, .label = label}, m_weapon(&weapon), visual{.sprite{svc.assets.projectile_textures.at(label) }} {

	auto const& in_data = svc.data.weapon["weapons"][id]["class_package"]["projectile"];

	metadata.type = static_cast<ProjectileType>(in_data["type"].as<int>());
	// package
	/*switch (metadata.type) {
	case ProjectileType::bullet: package.bullet = std::make_unique<BulletPackage>(svc, svc.data.weapon["weapons"][id]["metadata"]["label"].as_string(), id, *this); break;
	case ProjectileType::missile: break;
	case ProjectileType::melee: break;
	}*/
	physical.bounding_box = shape::Shape({in_data["dimensions"][0].as<float>(), in_data["dimensions"][1].as<float>()});
	visual.sprite.set_dimensions({in_data["dimensions"][0].as<int>(), in_data["dimensions"][1].as<int>()});

	metadata.specifications.base_damage = in_data["attributes"]["base_damage"].as<float>();
	metadata.specifications.power = in_data["attributes"]["power"] ? in_data["attributes"]["power"].as<int>() : 1;
	metadata.specifications.speed = in_data["attributes"]["speed"].as<float>();
	metadata.specifications.speed_variance = in_data["attributes"]["speed_variance"].as<float>();
	metadata.specifications.speed += svc.random.random_range_float(-metadata.specifications.speed_variance, metadata.specifications.speed_variance);
	metadata.specifications.variance = in_data["attributes"]["variance"].as<float>();
	metadata.specifications.stun_time = in_data["attributes"]["stun_time"].as<float>();
	metadata.specifications.knockback = in_data["attributes"]["knockback"].as<float>();
	metadata.specifications.acceleration_factor = in_data["attributes"]["acceleration_factor"].as<float>();
	metadata.specifications.dampen_factor = in_data["attributes"]["dampen_factor"].as<float>();
	metadata.specifications.dampen_variance = in_data["attributes"]["dampen_variance"].as<float>();

	if (static_cast<bool>(in_data["attributes"]["persistent"].as_bool())) { metadata.attributes.set(ProjectileAttributes::persistent); }
	if (static_cast<bool>(in_data["attributes"]["transcendent"].as_bool())) { metadata.attributes.set(ProjectileAttributes::transcendent); }
	if (static_cast<bool>(in_data["attributes"]["constrained"].as_bool())) { metadata.attributes.set(ProjectileAttributes::constrained); }
	if (static_cast<bool>(in_data["attributes"]["omnidirectional"].as_bool())) { metadata.attributes.set(ProjectileAttributes::omnidirectional); }

	visual.sprite.push_params("anim", {0, in_data["animation"]["num_frames"].as<int>(), in_data["animation"]["framerate"].as<int>()});
	visual.sprite.set_params("anim");
	visual.sprite.set_origin({0.f, static_cast<float>(visual.dimensions.y) * 0.5f});
	visual.effect_type = in_data["visual"]["effect_type"].as<int>();
	visual.render_type = visual.sprite.size() > 1 ? RenderType::multi_sprite : RenderType::single_sprite;

	metadata.specifications.lifespan = in_data["attributes"]["lifespan"].as<int>();
	metadata.specifications.lifespan_variance = in_data["attributes"]["lifespan_variance"].as<int>();
	auto var = svc.random.random_range(-metadata.specifications.lifespan_variance, metadata.specifications.lifespan_variance);
	lifetime = util::Cooldown{metadata.specifications.lifespan + var};

	physical.physics = components::PhysicsComponent({1.0f, 1.0f}, 1.0f);
	physical.physics.velocity.x = metadata.specifications.speed;
	if (metadata.specifications.dampen_factor != 0.f) {
		auto var = svc.random.random_range_float(-metadata.specifications.dampen_variance, metadata.specifications.dampen_variance);
		physical.physics.set_global_friction(metadata.specifications.dampen_factor + var);
	}

	// circle
	if (metadata.attributes.test(ProjectileAttributes::circle)) { physical.sensor = components::CircleSensor(physical.bounding_box.dimensions.x * 0.5f); }

	physical.max_dimensions = physical.bounding_box.dimensions;

	variables.state.set(ProjectileState::initialized);
	cooldown.start(40);
	lifetime.start();
	seed(svc);
}

void Projectile::update(automa::ServiceProvider& svc, player::Player& player) {

	// animation
	visual.sprite.update(physical.physics.position);
	if (physical.sensor) { visual.sprite.update(physical.sensor.value().bounds.getPosition()); }
	cooldown.update();
	lifetime.update();
	if (variables.state.test(ProjectileState::destruction_initiated) && !metadata.attributes.test(ProjectileAttributes::constrained)) { destroy(true); }

	physical.physics.update_euler(svc);

	if (physical.direction.lr == dir::LR::left) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x, physical.physics.position.y - physical.bounding_box.dimensions.y / 2});
	} else if (physical.direction.lr == dir::LR::right) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.dimensions.x, physical.physics.position.y - physical.bounding_box.dimensions.y / 2});
	} else if (physical.direction.und == dir::UND::up) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.dimensions.x / 2, physical.physics.position.y});
	} else if (physical.direction.und == dir::UND::down) {
		physical.bounding_box.set_position(shape::Shape::Vec{physical.physics.position.x - physical.bounding_box.dimensions.x / 2, physical.physics.position.y - physical.bounding_box.dimensions.y});
	}

	if (physical.sensor) { physical.sensor.value().set_position(physical.bounding_box.position); }

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
			svc.soundboard.flags.world.set(audio::World::soft_tap);
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

	// this is the right idea but needs to be refactored and generalized
	visual.sprite.render(svc, win, cam);

	// proj bounding box for debug
	if (svc.greyblock_mode()) {
		sf::RectangleShape box{};
		box.setSize(physical.bounding_box.dimensions);
		if (variables.state.test(ProjectileState::destruction_initiated)) {
			box.setFillColor(sf::Color{255, 255, 60, 160});
		} else {
			box.setFillColor(sf::Color{255, 255, 255, 160});
		}
		box.setPosition(physical.bounding_box.position - cam);
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
			variables.destruction_point = physical.bounding_box.position;
		} else {
			variables.destruction_point = physical.bounding_box.position + physical.bounding_box.dimensions;
		}
		variables.state.set(ProjectileState::destruction_initiated);
	}

	metadata.specifications.base_damage = 0;
}

void Projectile::seed(automa::ServiceProvider& svc, sf::Vector2<float> target) {
	float var = svc.random.random_range_float(-metadata.specifications.variance, metadata.specifications.variance);
	if (omnidirectional()) {
		physical.physics.velocity = util::unit(target) * metadata.specifications.speed;
		return;
	}
	switch (physical.direction.lr) {
	case dir::LR::left: physical.physics.velocity = {-metadata.specifications.speed, var}; break;
	case dir::LR::right: physical.physics.velocity = {metadata.specifications.speed, var}; break;
	}
	switch (physical.direction.und) {
	case dir::UND::up: physical.physics.velocity = {var, -metadata.specifications.speed}; break;
	case dir::UND::down: physical.physics.velocity = {var, metadata.specifications.speed}; break;
	}
}

void Projectile::set_position(sf::Vector2<float> pos) {
	physical.physics.position = pos;
	physical.bounding_box.position = pos;
	variables.fired_point = pos;
}

void Projectile::set_team(Team to_team) { metadata.team = to_team; }

void Projectile::set_firing_direction(dir::Direction to_direction) { physical.direction = to_direction; }

void Projectile::poof() { variables.state.set(arms::ProjectileState::poof); }

} // namespace arms
