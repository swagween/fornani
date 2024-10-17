
#include "Drop.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../level/Map.hpp"

namespace item {

Drop::Drop(automa::ServiceProvider& svc, std::string_view key, float probability, int delay_time, int special_id) : sparkler(svc, drop_dimensions, svc.styles.colors.ui_white, "drop"), special_id(special_id) {

	collider = shape::Collider(drop_dimensions);
	collider.sync_components();
	collider.physics.elasticity = 0.8f;

	sprite_dimensions.x = svc.data.drop[key]["sprite_dimensions"][0].as<float>();
	sprite_dimensions.y = svc.data.drop[key]["sprite_dimensions"][1].as<float>();
	spritesheet_dimensions.x = svc.data.drop[key]["spritesheet_dimensions"][0].as<int>();
	spritesheet_dimensions.y = svc.data.drop[key]["spritesheet_dimensions"][1].as<int>();

	type = static_cast<DropType>(svc.data.drop[key]["type"].as<int>());
	if (type == DropType::gem) { collider.physics.elasticity = 1.f; }

	collider.physics.set_global_friction(svc.data.drop[key]["friction"].as<float>());
	collider.stats.GRAV = svc.data.drop[key]["gravity"].as<float>();
	collider.physics.maximum_velocity = {32.f, 32.f};

	auto& in_anim = svc.data.drop[key]["animation"];
	num_sprites = in_anim["num_sprites"].as<int>();
	for (auto& param : in_anim["params"].array_view()) {
		anim::Parameters a{};
		a.duration = param["duration"].as<int>();
		a.framerate = param["framerate"].as<int>();
		a.num_loops = param["num_loops"].as<int>();
		sprite.push_params(param["label"].as_string(), a);
	}
	sprite.set_params("neutral", true);
	sf::Vector2<int> dim{static_cast<int>(sprite_dimensions.x), static_cast<int>(sprite_dimensions.y)};
	sprite.set_dimensions(dim);

	// randomly seed the animation start frame so drops in the same loot animate out of sync
	sprite.random_start(svc);

	auto rand_cooldown_offset = svc.random.random_range(0, 50);
	auto rand_shine_offset = svc.random.random_range(0, 600);
	if (type == DropType::gem) { rand_cooldown_offset += 2000; }
	lifespan.start(4500 + rand_cooldown_offset);
	shine_cooldown.start(shine_cooldown.get_native_time() + rand_shine_offset);
	delay.start(delay_time);
	seed(svc, probability);
	set_value();
	set_texture(svc);

	sparkler.set_dimensions(collider.bounding_box.dimensions);
}

void Drop::seed(automa::ServiceProvider& svc, float probability) {
	auto random_sample = svc.random.random_range_float(0.0f, 1.0f);
	if (random_sample < probability * constants.priceless) {
		rarity = priceless;
	} else if (random_sample < probability * constants.rare) {
		rarity = rare;
	} else if (random_sample < probability * constants.uncommon) {
		rarity = uncommon;
	} else {
		rarity = common;
	}
}

void Drop::set_value() {
	// heart
	if (type == DropType::heart) {
		if (rarity == priceless || rarity == rare) {
			value = 3;
		} else {
			value = 1;
		}
		return;
	}

	// orb
	switch (rarity) {
	case priceless: value = 100; break;
	case rare: value = 20; break;
	case uncommon: value = 5; break;
	case common: value = 1; break;
	}

	//gem
	if (type == DropType::gem) { value = special_id; }
}

void Drop::set_texture(automa::ServiceProvider& svc) {
	switch (type) {
	case DropType::heart: sprite.set_texture(svc.assets.t_heart); break;
	case DropType::orb: sprite.set_texture(svc.assets.t_orb); break;
	case DropType::gem: sprite.set_texture(svc.assets.t_gem); break;
	}
}

void Drop::update(automa::ServiceProvider& svc, world::Map& map) {
	delay.update();
	collider.update(svc);
	collider.detect_map_collision(map);
	for (auto& breakable : map.breakables) { collider.handle_collider_collision(breakable.get_bounding_box()); }
	for (auto& pushable : map.pushables) { collider.handle_collider_collision(pushable.get_bounding_box()); }
	for (auto& platform : map.platforms) { collider.handle_collider_collision(platform.bounding_box); }
	for (auto& block : map.switch_blocks) {
		if (block.on()) { collider.handle_collider_collision(block.get_bounding_box()); }
	}
	for (auto& destroyer : map.destroyers) { collider.handle_collider_collision(destroyer.get_bounding_box()); }
	for (auto& spike : map.spikes) { collider.handle_collider_collision(spike.get_bounding_box()); }
	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};
	if (collider.flags.external_state.test(shape::ExternalState::world_collision) && type == DropType::gem && !is_inactive() && abs(collider.physics.velocity.y) > 1.f) { svc.soundboard.flags.world.set(audio::World::wall_hit); }

	lifespan.update();
	afterlife.update();

	sparkler.update(svc);
	sparkler.set_position(collider.bounding_box.position);

	sprite_offset = {0.f, static_cast<float>(drop_dimensions.y - sprite_dimensions.y) * 0.5f};

	int u{};
	int v{};
	if (type == DropType::heart) { v = rarity == priceless || rarity == rare ? 1 : 0; }
	if (type == DropType::orb) { v = rarity == priceless ? 3 : (rarity == rare ? 2 : (rarity == uncommon ? 1 : 0)); }
	if (type == DropType::gem) { v = special_id; }
	sprite.set_origin(sprite_dimensions * 0.5f);
	sprite.update(collider.get_center() + sprite_offset, u, v, true);

	state_function = state_function();
}

void Drop::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
	} else {
		if (!is_inactive() && !is_completely_gone() && (lifespan.get_cooldown() > 500 || (lifespan.get_cooldown() / 20) % 2 == 0)) { sprite.render(svc, win, cam); }
		sparkler.render(svc, win, cam);
	}
}

void Drop::set_position(sf::Vector2<float> pos) {
	collider.physics.position = pos;
	sparkler.set_position(pos);
}

void Drop::destroy_completely() {
	lifespan.cancel();
	afterlife.cancel();
}

void Drop::deactivate() {
	lifespan.cancel();
	afterlife.start(1000);
	sparkler.deactivate();
}

shape::Collider& Drop::get_collider() { return collider; }

DropType Drop::get_type() const { return type; }

int Drop::get_value() const { return value; }

fsm::StateFunction Drop::update_neutral() {
	shine_cooldown.update();
	if (sprite.size() > 1 && shine_cooldown.is_complete()) {
		sprite.set_params("shine", true);
		return DROP_BIND(update_shining);
	}
	return DROP_BIND(update_neutral);
}

fsm::StateFunction Drop::update_shining() {
	if (sprite.complete()) {
		sprite.set_params("neutral", true);
		shine_cooldown.start();
		return DROP_BIND(update_neutral);
	}
	return DROP_BIND(update_shining);
}

bool Drop::is_completely_gone() const { return afterlife.is_complete() && lifespan.is_complete(); }

bool Drop::is_inactive() const { return lifespan.is_complete() && !afterlife.is_complete(); }

} // namespace item
