#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::enemy {

Enemy::Enemy(automa::ServiceProvider& svc, std::string_view label, bool spawned, int variant, sf::Vector2<int> start_direction)
	: entity::Entity(svc), label(label), health_indicator(svc), directions{.actual{start_direction}, .desired{start_direction}}, visual{.sprite = sf::Sprite(svc.assets.texture_lookup.at(label))} {

	direction = dir::Direction{start_direction};

	if (spawned) { flags.general.set(GeneralFlags::spawned); }

	auto const& in_data = svc.data.enemy[label];
	auto const& in_metadata = in_data["metadata"];
	auto const& in_physical = in_data["physical"];
	auto const& in_attributes = in_data["attributes"];
	auto const& in_visual = in_data["visual"];
	auto const& in_audio = in_data["audio"];
	auto const& in_animation = in_data["animation"];
	auto const& in_general = in_data["general"];

	dimensions.x = in_physical["dimensions"][0].as<float>();
	dimensions.y = in_physical["dimensions"][1].as<float>();

	collider = shape::Collider(dimensions);
	collider.sync_components();
	collider.physics.set_global_friction(in_physical["friction"].as<float>());
	collider.stats.GRAV = in_physical["gravity"].as<float>();

	secondary_collider = shape::Collider(dimensions);
	secondary_collider.sync_components();
	secondary_collider.physics.set_global_friction(in_physical["friction"].as<float>());
	secondary_collider.stats.GRAV = in_physical["gravity"].as<float>();

	metadata.id = in_metadata["id"].as<int>();
	metadata.variant = in_metadata["variant"].as_string();

	sprite_dimensions.x = in_physical["sprite_dimensions"][0].as<int>();
	sprite_dimensions.y = in_physical["sprite_dimensions"][1].as<int>();
	spritesheet_dimensions.x = in_physical["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = in_physical["spritesheet_dimensions"][1].as<int>();
	sprite_offset.x = in_physical["offset"][0].as<float>();
	sprite_offset.y = in_physical["offset"][1].as<float>();

	// TODO: load hurtboxes and colliders

	physical.alert_range.set_dimensions({in_physical["alert_range"][0].as<float>(), in_physical["alert_range"][1].as<float>()});
	physical.hostile_range.set_dimensions({in_physical["hostile_range"][0].as<float>(), in_physical["hostile_range"][1].as<float>()});

	attributes.base_damage = in_attributes["base_damage"].as<float>();
	attributes.base_hp = in_attributes["base_hp"].as<float>();
	attributes.loot_multiplier = in_attributes["loot_multiplier"].as<float>();
	attributes.speed = in_attributes["speed"].as<float>();
	attributes.drop_range.x = in_attributes["drop_range"][0].as<int>();
	attributes.drop_range.y = in_attributes["drop_range"][1].as<int>();
	attributes.rare_drop_id = in_attributes["rare_drop_id"].as<int>();
	attributes.respawn_distance = in_attributes["respawn_distance"].as<int>();
	if (in_attributes["permadeath"].as_bool()) { flags.general.set(GeneralFlags::permadeath); };

	visual.effect_size = in_visual["effect_size"].as<int>();
	visual.effect_type = in_visual["effect_type"].as<int>();
	// TODO: load in all the animation data and map them to a set of parameters
	// let's add this function to services
	anim::Parameters params{};
	params.duration = in_animation["duration"].as<int>();
	params.framerate = in_animation["framerate"].as<int>();
	animation.set_params(params);

	switch (in_audio["hit"].as<int>()) {
	case -1: flags.general.set(GeneralFlags::custom_sounds); break;
	case 0: sound.hit_flag = audio::Enemy::hit_low; break;
	case 1: sound.hit_flag = audio::Enemy::hit_medium; break;
	case 2: sound.hit_flag = audio::Enemy::hit_high; break;
	case 3: sound.hit_flag = audio::Enemy::hit_squeak; break;
	}

	health.set_max(attributes.base_hp);
	health_indicator.init(svc, 0);
	post_death.start(afterlife);

	if (in_general["mobile"].as_bool()) { flags.general.set(GeneralFlags::mobile); }
	if (in_general["gravity"].as_bool()) { flags.general.set(GeneralFlags::gravity); }
	if (in_general["map_collision"].as_bool()) { flags.general.set(GeneralFlags::map_collision); }
	if (in_general["player_collision"].as_bool()) { flags.general.set(GeneralFlags::player_collision); }
	if (in_general["hurt_on_contact"].as_bool()) { flags.general.set(GeneralFlags::hurt_on_contact); }
	if (in_general["uncrushable"].as_bool()) { flags.general.set(GeneralFlags::uncrushable); }
	if (in_general["foreground"].as_bool()) { flags.general.set(GeneralFlags::foreground); }
	if (in_general["rare_drops"].as_bool()) { flags.general.set(GeneralFlags::rare_drops); }
	if (!flags.general.test(GeneralFlags::gravity)) { collider.stats.GRAV = 0.f; }
	if (!flags.general.test(GeneralFlags::uncrushable)) { collider.collision_depths = util::CollisionDepth(); }

	drawbox.setSize({static_cast<float>(sprite_dimensions.x), static_cast<float>(sprite_dimensions.y)});
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.ui_white);
	drawbox.setOutlineThickness(-1);
}

void Enemy::set_external_id(std::pair<int, sf::Vector2<int>> code) { 
	// TODO: find a better way to generate unique external IDs
	metadata.external_id = code.first * 2719 + code.second.x * 13219 + code.second.y * 49037;
}

void Enemy::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	
	if (collider.collision_depths) { collider.collision_depths.value().reset(); }
	sound.hurt_sound_cooldown.update();
	if (just_died()) { svc.data.kill_enemy(map.room_id, metadata.external_id, attributes.respawn_distance, permadeath()); }
	if (just_died() && !flags.state.test(StateFlags::special_death_mode)) {
		svc.stats.enemy.enemies_killed.update();
		map.active_loot.push_back(item::Loot(svc, attributes.drop_range, attributes.loot_multiplier, collider.get_center(), 0, flags.general.test(GeneralFlags::rare_drops), attributes.rare_drop_id));
		svc.soundboard.flags.frdog.set(audio::Frdog::death);
		map.spawn_counter.update(-1);
	}
	flags.triggers = {};
	if (map.off_the_bottom(collider.physics.position)) {
		if (svc.ticker.every_x_ticks(10)) { health.inflict(4.f); }
	}
	if (just_died() && !flags.general.test(GeneralFlags::post_death_render)) { map.effects.push_back(entity::Effect(svc, collider.physics.position, collider.physics.velocity, visual.effect_type, visual.effect_size)); }
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) {
		health_indicator.update(svc, collider.physics.position);
		post_death.update();
		return;
	}
	// shake
	energy = std::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = util::Random::random_vector_float(-energy, energy); }
	if (hitstun.running()) {
		hitstun.update();
		if (svc.ticker.every_x_ticks(4)) {
		} else {
			return;
		}
	}

	// stuff that slows down from hitstun
	Entity::update(svc, map);
	collider.update(svc);
	secondary_collider.update(svc);
	health_indicator.update(svc, collider.physics.position);

	if (flags.general.test(GeneralFlags::map_collision)) {
		for (auto& breakable : map.breakables) { breakable.handle_collision(collider); }
		collider.detect_map_collision(map);
		secondary_collider.detect_map_collision(map);
	}
	for (auto& other : map.enemy_catalog.enemies) {
		if (other.get() != this) { handle_collision(other->collider); }
	}

	collider.reset();
	secondary_collider.reset();
	if (collider.collision_depths) { collider.collision_depths.value().update(); }
	collider.reset_ground_flags();
	secondary_collider.reset_ground_flags();
	collider.physics.acceleration = {};
	secondary_collider.physics.acceleration = {};

	animation.update();
	health.update();

	// update ranges
	physical.alert_range.set_position(collider.bounding_box.get_position() - (physical.alert_range.get_dimensions() * 0.5f) + (collider.dimensions * 0.5f));
	physical.hostile_range.set_position(collider.bounding_box.get_position() - (physical.hostile_range.get_dimensions() * 0.5f) + (collider.dimensions * 0.5f));
	if (player.collider.bounding_box.overlaps(physical.alert_range)) {
		if (!alert()) { flags.triggers.set(Triggers::alert); }
		flags.state.set(StateFlags::alert);
	} else {
		flags.state.reset(StateFlags::alert);
	}
	if (player.collider.bounding_box.overlaps(physical.hostile_range)) {
		if (!hostile()) { flags.triggers.set(Triggers::hostile); }
		flags.state.set(StateFlags::hostile);
	} else {
		flags.state.reset(StateFlags::hostile);
	}

	// animate
	auto column{0};
	if (hurt_effect.running()) { column = (hurt_effect.get_cooldown() / 32) % 2 == 0 ? 1 : 2; }
	if (hurt_effect.running()) { shake(); }
	auto u = column * sprite_dimensions.x;
	auto v = static_cast<int>(animation.get_frame()) * sprite_dimensions.y;
	visual.sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
	visual.sprite.setOrigin({static_cast<float>(sprite_dimensions.x) * 0.5f, static_cast<float>(dimensions.y) * 0.5f});
}

void Enemy::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) { handle_player_collision(player); }

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	auto sprite_position = collider.physics.position + sprite_offset - cam + random_offset;

	// exit conditions
	if (!svc.in_window(sprite_position, visual.sprite.getGlobalBounds().size)) { return; }
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) { return; }
	if (flags.state.test(StateFlags::invisible)) { return; }

	drawbox.setOrigin(visual.sprite.getOrigin());
	drawbox.setPosition(collider.physics.position + sprite_offset - cam);
	visual.sprite.setPosition(sprite_position);
	win.draw(visual.sprite);
	if (svc.greyblock_mode()) {
		win.draw(visual.sprite);
		drawbox.setOrigin({});
		drawbox.setSize(collider.hurtbox.get_dimensions());
		drawbox.setOutlineColor(svc.styles.colors.ui_white);
		drawbox.setPosition(collider.hurtbox.get_position() - cam);
		win.draw(drawbox);
		drawbox.setPosition(physical.alert_range.get_position() - cam);
		drawbox.setSize(physical.alert_range.get_dimensions());
		drawbox.setOutlineColor(sf::Color{80, 20, 60, 80});
		win.draw(drawbox);
		drawbox.setPosition(physical.hostile_range.get_position() - cam);
		drawbox.setSize(physical.hostile_range.get_dimensions());
		drawbox.setOutlineColor(sf::Color{140, 30, 60, 110});
		win.draw(drawbox);
		collider.render(win, cam);
		secondary_collider.render(win, cam);
		health.render(svc, win, cam);
	}
}

void Enemy::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (flags.state.test(StateFlags::invisible)) { return; }
	health_indicator.render(svc, win, cam);
}

void Enemy::handle_player_collision(player::Player& player) const {
	if (died()) { return; }
	if (player_collision()) { player.collider.handle_collider_collision(collider); }
	if (flags.general.test(GeneralFlags::hurt_on_contact)) {
		if (player.collider.hurtbox.overlaps(collider.bounding_box)) { player.hurt(attributes.base_damage); }
	}
}

void Enemy::handle_collision(shape::Collider& other) { collider.handle_collider_collision(other, true); }

void Enemy::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.get_team() == arms::Team::skycorps) { return; }
	if (proj.get_team() == arms::Team::guardian) { return; }
	if (proj.get_team() == arms::Team::beast) { return; }
	if (flags.state.test(StateFlags::invisible)) { return; }
	if (!(proj.get_bounding_box().overlaps(collider.bounding_box) || proj.get_bounding_box().overlaps(secondary_collider.bounding_box))) { return; }
	flags.state.set(enemy::StateFlags::shot);
	if (flags.state.test(enemy::StateFlags::vulnerable) && !died()) {
		if (proj.persistent()) { proj.damage_over_time(); }
		if (proj.can_damage()) {
			hurt();
			health.inflict(proj.get_damage());
			health_indicator.add(-proj.get_damage());
			if (!flags.general.test(GeneralFlags::custom_sounds) && !sound.hurt_sound_cooldown.running()) { svc.soundboard.flags.enemy.set(sound.hit_flag); }
			map.effects.push_back(entity::Effect(svc, proj.get_position(), {}, 0, 11, {1, 1}));
			hitstun.start(64);
		}
	} else if (!flags.state.test(enemy::StateFlags::vulnerable)) {
		map.effects.push_back(entity::Effect(svc, proj.get_position(), {}, 0, 6));
		svc.soundboard.flags.world.set(audio::World::hard_hit);
	}
	if (!proj.persistent() && (!died() || just_died())) { proj.destroy(false); }
}

void Enemy::on_crush(world::Map& map) {
	if (!collider.collision_depths) { return; }
	if (flags.general.test(GeneralFlags::uncrushable)) { return; }
	if (collider.crushed() || secondary_collider.crushed()) {
		hurt();
		health.inflict(1024.f);
		health_indicator.add(-1024.f);
		collider.collision_depths = {};
	}
}

bool Enemy::player_behind(player::Player& player) const { return player.collider.physics.position.x + player.collider.bounding_box.get_dimensions().x * 0.5f < collider.physics.position.x + collider.dimensions.x * 0.5f; }

void Enemy::set_position_from_scaled(sf::Vector2<float> pos) {
	auto new_pos = pos;
	auto round = static_cast<int>(collider.dimensions.y) % 32;
	new_pos.y += static_cast<float>(32.f - round);
	set_position(new_pos);
}

} // namespace fornani::enemy
