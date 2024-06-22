#include "Enemy.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../player/Player.hpp"
#include "../../level/Map.hpp"

namespace enemy {

Enemy::Enemy(automa::ServiceProvider& svc, std::string_view label) : entity::Entity(svc), label(label), health_indicator(svc) {
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
	sprite_offset.x = in_physical["offset"][0].as<int>();
	sprite_offset.y = in_physical["offset"][1].as<int>();

	// TODO: load hurtboxes and colliders

	physical.alert_range.dimensions.x = in_physical["alert_range"][0].as<float>();
	physical.alert_range.dimensions.y = in_physical["alert_range"][1].as<float>();
	physical.hostile_range.dimensions.x = in_physical["hostile_range"][0].as<float>();
	physical.hostile_range.dimensions.y = in_physical["hostile_range"][1].as<float>();

	attributes.base_damage = in_attributes["base_damage"].as<int>();
	attributes.base_hp = in_attributes["base_hp"].as<float>();
	attributes.loot_multiplier = in_attributes["loot_multiplier"].as<float>();
	attributes.speed = in_attributes["speed"].as<float>();
	attributes.drop_range.x = in_attributes["drop_range"][0].as<int>();
	attributes.drop_range.y = in_attributes["drop_range"][1].as<int>();

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
	case 0: sounds.hit.setBuffer(svc.assets.b_enemy_hit_low); break;
	case 1: sounds.hit.setBuffer(svc.assets.b_enemy_hit_medium); break;
	case 2: sounds.hit.setBuffer(svc.assets.b_enemy_hit_high); break;
	case 3: sounds.hit.setBuffer(svc.assets.b_enemy_hit_squeak); break;
	}
	sounds.inv_hit.setBuffer(svc.assets.b_enemy_hit_inv);

	health.set_max(attributes.base_hp);
	health_indicator.init(svc, 0);
	post_death.start(afterlife);

	direction.lr = dir::LR::left;

	if (in_general["mobile"].as_bool()) { flags.general.set(GeneralFlags::mobile); }
	if (in_general["gravity"].as_bool()) { flags.general.set(GeneralFlags::gravity); }
	if (in_general["map_collision"].as_bool()) { flags.general.set(GeneralFlags::map_collision); }
	if (in_general["player_collision"].as_bool()) { flags.general.set(GeneralFlags::player_collision); }
	if (in_general["hurt_on_contact"].as_bool()) { flags.general.set(GeneralFlags::hurt_on_contact); }
	if (!flags.general.test(GeneralFlags::gravity)) { collider.stats.GRAV = 0.f; }

	sprite.setTexture(svc.assets.texture_lookup.at(label));
	drawbox.setSize({(float)sprite_dimensions.x, (float)sprite_dimensions.y});
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineColor(svc.styles.colors.ui_white);
	drawbox.setOutlineThickness(-1);
}

void Enemy::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	flags.triggers = {};
	if (map.off_the_bottom(collider.physics.position)) {
		if (svc.ticker.every_x_ticks(10)) { health.inflict(4); }
	}
	if (just_died() && !flags.general.test(GeneralFlags::post_death_render)) { map.effects.push_back(entity::Effect(svc, collider.physics.position, collider.physics.velocity, visual.effect_type, visual.effect_size)); }
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) {
		health_indicator.update(svc, collider.physics.position);
		post_death.update();
		return;
	}
	Entity::update(svc, map);
	collider.update(svc);
	secondary_collider.update(svc);
	health_indicator.update(svc, collider.physics.position);
	if (flags.general.test(GeneralFlags::map_collision)) {
		for (auto& breakable : map.breakables) { breakable.handle_collision(collider); }
		collider.detect_map_collision(map);
		secondary_collider.detect_map_collision(map);
	}
	collider.reset();
	secondary_collider.reset();
	collider.reset_ground_flags();
	secondary_collider.reset_ground_flags();
	collider.physics.acceleration = {};
	secondary_collider.physics.acceleration = {};

	animation.update();
	health.update();

	//update ranges
	physical.alert_range.set_position(collider.bounding_box.position - (physical.alert_range.dimensions * 0.5f) + (collider.dimensions * 0.5f));
	physical.hostile_range.set_position(collider.bounding_box.position - (physical.hostile_range.dimensions * 0.5f) + (collider.dimensions * 0.5f));
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

	// get UV coords
	if (spritesheet_dimensions.y != 0) {
		int u = (int)(animation.get_frame() / spritesheet_dimensions.y) * sprite_dimensions.x;
		int v = (int)(animation.get_frame() % spritesheet_dimensions.y) * sprite_dimensions.y;
		sprite.setTextureRect(sf::IntRect({u, v}, {sprite_dimensions.x, sprite_dimensions.y}));
	}
	sprite.setOrigin((float)sprite_dimensions.x / 2.f, (float)dimensions.y / 2.f);
}

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) { return; }
	drawbox.setOrigin(sprite.getOrigin());
	drawbox.setPosition(collider.physics.position + sprite_offset - cam);
	sprite.setPosition(collider.physics.position + sprite_offset - cam + random_offset);
	if (!flags.state.test(StateFlags::shaking)) { random_offset = {}; }
	if (svc.greyblock_mode()) {
		drawbox.setOrigin({0.f, 0.f});
		drawbox.setSize({(float)collider.hurtbox.dimensions.x, (float)collider.hurtbox.dimensions.y});
		drawbox.setOutlineColor(svc.styles.colors.ui_white);
		drawbox.setPosition(collider.hurtbox.position - cam);
		win.draw(drawbox);
		drawbox.setPosition(physical.alert_range.position - cam);
		drawbox.setSize(physical.alert_range.dimensions);
		drawbox.setOutlineColor(sf::Color{80, 20, 60, 80});
		win.draw(drawbox);
		drawbox.setPosition(physical.hostile_range.position - cam);
		drawbox.setSize(physical.hostile_range.dimensions);
		drawbox.setOutlineColor(sf::Color{140, 30, 60, 110});
		win.draw(drawbox);
		collider.render(win, cam);
		health.render(svc, win, cam);
	} else {
		win.draw(sprite);
	}
}

void Enemy::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) { health_indicator.render(svc, win, cam); }

void Enemy::handle_player_collision(player::Player& player) const {
	if (died()) { return; }
	if (player_collision()) { player.collider.handle_collider_collision(collider.bounding_box); }
	if (flags.general.test(GeneralFlags::hurt_on_contact)) {
		if (player.collider.hurtbox.overlaps(collider.bounding_box)) { player.hurt(attributes.base_damage); }
	}
}

void Enemy::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {

	if (proj.team == arms::TEAMS::SKYCORPS) { return; }
	if (!(proj.bounding_box.overlaps(collider.bounding_box) || proj.bounding_box.overlaps(secondary_collider.bounding_box))) { return; }

	if (svc.ticker.every_x_ticks(10)) { proj.multiply(1.2f); }

	flags.state.set(enemy::StateFlags::shot);
	if (flags.state.test(enemy::StateFlags::vulnerable) && !died()) {
		hurt();
		health.inflict(proj.get_damage());
		health_indicator.add(-proj.get_damage());

		if (!flags.general.test(GeneralFlags::custom_sounds)) { sounds.hit.play(); }

		if (just_died()) {
			map.active_loot.push_back(item::Loot(svc, attributes.drop_range, attributes.loot_multiplier, collider.bounding_box.position));
			svc.soundboard.flags.frdog.set(audio::Frdog::death);
		}
	} else if (!flags.state.test(enemy::StateFlags::vulnerable)) {
		map.effects.push_back(entity::Effect(svc, proj.physics.position, {}, 0, 6));
		sounds.inv_hit.play();
	}
	if (!proj.stats.persistent && (!died() || just_died())) { proj.destroy(false); }
}

} // namespace enemy