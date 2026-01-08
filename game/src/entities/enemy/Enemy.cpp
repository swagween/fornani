
#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

#include <imgui.h>
#include <ccmath/ext/clamp.hpp>
#include <numbers>

namespace fornani::enemy {

Enemy::Enemy(automa::ServiceProvider& svc, world::Map& map, std::string_view label, bool spawned, int variant, sf::Vector2<int> start_direction)
	: Mobile(svc, map, "enemy_" + std::string{label}, sf::Vector2i{svc.data.enemy[label]["physical"]["sprite_dimensions"][0].as<int>(), svc.data.enemy[label]["physical"]["sprite_dimensions"][1].as<int>()}), metadata{.variant{variant}},
	  label(label), health_indicator{svc}, hurt_effect{128}, m_health_bar{svc, colors::mythic_green}, health{svc.data.enemy[label]["attributes"]["base_hp"].as<float>()} {

	get_collider().set_trait(shape::ColliderTrait::enemy);
	if (spawned) { flags.general.set(GeneralFlags::spawned); }
	directions.actual = Direction{start_direction};
	directions.desired = Direction{start_direction};

	auto const& in_data = svc.data.enemy[label];
	auto const& in_metadata = in_data["metadata"];
	auto const& in_physical = in_data["physical"];
	auto const& in_attributes = in_data["attributes"];
	auto const& in_visual = in_data["visual"];
	auto const& in_audio = in_data["audio"];
	auto const& in_animation = in_data["animation"];
	auto const& in_general = in_data["general"];
	auto const& in_treasure = in_data["treasure"];

	auto dimensions = sf::Vector2f{in_physical["dimensions"][0].as<float>(), in_physical["dimensions"][1].as<float>()};

	get_collider().set_dimensions(dimensions);
	get_collider().physics.set_global_friction(in_physical["friction"].as<float>());
	get_collider().flags.general.set(shape::General::complex);
	get_collider().clear_chunks();
	attributes.gravity = in_physical["gravity"].as<float>();

	m_native_offset = sf::Vector2f{in_physical["offset"][0].as<float>(), in_physical["offset"][1].as<float>()};

	metadata.id = in_metadata["id"].as<int>();

	physical.alert_range.set_dimensions({in_physical["alert_range"][0].as<float>(), in_physical["alert_range"][1].as<float>()});
	physical.hostile_range.set_dimensions({in_physical["hostile_range"][0].as<float>(), in_physical["hostile_range"][1].as<float>()});
	physical.home_detector.set_dimensions({in_physical["home_detector"][0].as<float>(), in_physical["home_detector"][1].as<float>()});

	attributes.base_damage = in_attributes["base_damage"].as<float>();
	attributes.base_hp = in_attributes["base_hp"].as<float>();
	attributes.loot_multiplier = in_attributes["loot_multiplier"].as<float>();
	attributes.speed = in_attributes["speed"].as<float>();
	attributes.drop_range.x = in_attributes["drop_range"][0].as<int>();
	attributes.drop_range.y = in_attributes["drop_range"][1].as<int>();
	attributes.rare_drop_id = in_attributes["rare_drop_id"].as<int>();
	attributes.respawn_distance = in_attributes["respawn_distance"].as<int>();
	if (in_attributes["permadeath"].as_bool()) { flags.general.set(GeneralFlags::permadeath); };
	if (in_attributes["size"]) { attributes.size = static_cast<EnemySize>(in_attributes["size"].as<int>()); }

	visual.effect_size = in_visual["effect_size"].as<int>();
	visual.effect_type = in_visual["effect_type"].as<int>();

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

	post_death.start(afterlife);

	if (in_general["mobile"].as_bool()) { flags.general.set(GeneralFlags::mobile); }
	if (in_general["gravity"].as_bool()) { flags.general.set(GeneralFlags::gravity); }
	if (in_general["map_collision"].as_bool()) { flags.general.set(GeneralFlags::map_collision); }
	if (in_general["player_collision"].as_bool()) { flags.general.set(GeneralFlags::player_collision); }
	if (in_general["hurt_on_contact"].as_bool()) { flags.general.set(GeneralFlags::hurt_on_contact); }
	if (in_general["uncrushable"].as_bool()) { flags.general.set(GeneralFlags::uncrushable); }
	if (in_general["foreground"].as_bool()) { flags.general.set(GeneralFlags::foreground); }
	if (in_general["rare_drops"].as_bool()) { flags.general.set(GeneralFlags::rare_drops); }
	if (in_general["spike_collision"].as_bool()) { flags.general.set(GeneralFlags::spike_collision); }
	if (in_general["sturdy"].as_bool()) { get_collider().set_attribute(shape::ColliderAttributes::sturdy); }
	if (in_general["crusher"].as_bool()) { get_collider().set_attribute(shape::ColliderAttributes::crusher); }
	if (in_general["fixed"].as_bool()) { get_collider().set_attribute(shape::ColliderAttributes::fixed); }
	if (in_general["semipermanent"].as_bool()) { flags.general.set(GeneralFlags::semipermanent); }
	if (in_general["no_tick"].as_bool()) { flags.general.set(GeneralFlags::no_tick); }
	if (!flags.general.test(GeneralFlags::gravity)) { get_collider().stats.GRAV = 0.f; }
	if (!flags.general.test(GeneralFlags::uncrushable)) { get_collider().collision_depths = util::CollisionDepth(); }

	if (!flags.general.test(GeneralFlags::player_collision)) { get_collider().set_exclusion_target(shape::ColliderTrait::player); }
	if (!flags.general.test(GeneralFlags::map_collision)) { get_collider().set_attribute(shape::ColliderAttributes::no_map_collision); }
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::secondary);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_soft_target(shape::ColliderTrait::enemy);
	if (get_collider().has_attribute(shape::ColliderAttributes::sturdy)) { get_collider().set_exclusion_target(shape::ColliderTrait::enemy); }

	if (in_general["secondary_collider"].as_bool()) {
		secondary_owned_collider.emplace(map, sf::Vector2f{dimensions});
		secondary_collider = *secondary_owned_collider;
		get_secondary_collider().physics.set_global_friction(in_physical["friction"].as<float>());
		get_secondary_collider().stats.GRAV = in_physical["gravity"].as<float>();
		get_secondary_collider().set_trait(shape::ColliderTrait::secondary);
		get_secondary_collider().set_trait(shape::ColliderTrait::enemy);
		if (!flags.general.test(GeneralFlags::player_collision)) { get_secondary_collider().set_exclusion_target(shape::ColliderTrait::player); }
		if (!flags.general.test(GeneralFlags::map_collision)) { get_secondary_collider().set_attribute(shape::ColliderAttributes::no_map_collision); }
		get_secondary_collider().set_exclusion_target(shape::ColliderTrait::circle);
		get_secondary_collider().set_exclusion_target(shape::ColliderTrait::npc);
		get_secondary_collider().set_resolution_exclusion_target(shape::ColliderTrait::player);
		get_secondary_collider().set_soft_target(shape::ColliderTrait::enemy);
		if (get_collider().has_attribute(shape::ColliderAttributes::sturdy)) {
			get_secondary_collider().set_exclusion_target(shape::ColliderTrait::enemy);
			get_secondary_collider().set_attribute(shape::ColliderAttributes::sturdy);
		}
		if (get_collider().has_attribute(shape::ColliderAttributes::crusher)) { get_secondary_collider().set_attribute(shape::ColliderAttributes::crusher); }
	}

	if (in_treasure.is_object()) {
		m_treasure = std::vector<Treasure>{};
		attributes.treasure_chance = in_treasure["chance"].as<float>();
		for (auto const& entry : in_treasure["items"].as_array()) { m_treasure->push_back(Treasure{entry["chance"].as<float>(), entry["tag"].as_string(), entry["mythic"].as_bool()}); }
	}

	center();
}

void Enemy::set_external_id(std::pair<int, sf::Vector2<int>> code) {
	// TODO: find a better way to generate unique external IDs
	metadata.external_id = code.first * 2719 + code.second.x * 13219 + code.second.y * 49037;
}

void Enemy::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	directions.desired.lnr = (player.get_collider().get_center().x < get_collider().get_center().x) ? LNR::left : LNR::right;
	directions.movement.lnr = get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;

	if (!flags.general.test(GeneralFlags::gravity)) {
		get_collider().stats.GRAV = 0.f;
	} else {
		get_collider().stats.GRAV = attributes.gravity;
	}
	if (has_secondary_collider()) { get_secondary_collider().stats.GRAV = get_collider().stats.GRAV; }

	intangibility.running() ? flags.state.set(StateFlags::intangible) : flags.state.reset(StateFlags::intangible);

	impulse.update();
	sound.hurt_sound_cooldown.update();
	intangibility.update();

	if (get_collider().collision_depths) { get_collider().collision_depths.value().reset(); }
	if (has_secondary_collider()) {
		if (get_secondary_collider().collision_depths) { get_secondary_collider().collision_depths.value().reset(); }
	}

	if (just_died()) { svc.data.kill_enemy(map.room_id, metadata.external_id, attributes.respawn_distance, permadeath(), flags.general.test(GeneralFlags::semipermanent)); }
	if (just_died() && !flags.state.test(StateFlags::special_death_mode)) {
		svc.stats.enemy.enemies_killed.update();
		map.active_loot.push_back(item::Loot(svc, map, player, attributes.drop_range, attributes.loot_multiplier, get_collider().get_center(), 0, flags.general.test(GeneralFlags::rare_drops), attributes.rare_drop_id));
		if (random::percent_chance(attributes.treasure_chance * 100.f)) { spawn_treasure(svc, map); }
		switch (attributes.size) {
		case EnemySize::tiny: svc.soundboard.flags.enemy.set(audio::Enemy::high_death); break;
		case EnemySize::small: svc.soundboard.flags.enemy.set(audio::Enemy::high_death); break;
		case EnemySize::medium: svc.soundboard.flags.enemy.set(audio::Enemy::standard_death); break;
		case EnemySize::large: svc.soundboard.flags.enemy.set(audio::Enemy::low_death); break;
		case EnemySize::giant: svc.soundboard.flags.enemy.set(audio::Enemy::low_death); break;
		default: svc.soundboard.flags.enemy.set(audio::Enemy::standard_death); break;
		}
		map.spawn_counter.update(-1);
		get_collider().set_flag(shape::ColliderFlags::intangible);
		get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	}
	flags.triggers = {};
	if (map.off_the_bottom(get_collider().physics.position)) {
		if (svc.ticker.every_x_ticks(10)) { health.inflict(4.f); }
	}
	if (just_died() && !flags.general.test(GeneralFlags::post_death_render)) {
		map.effects.push_back(entity::Effect(svc, "large_explosion", get_collider().get_center(), get_collider().physics.apparent_velocity() * 0.5f, visual.effect_type));
	}
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) {
		health_indicator.update(svc, get_collider().physics.position);
		post_death.update();
		return;
	}

	health.update();
	m_health_bar.update(health.get_normalized(), get_collider().get_top() + sf::Vector2f{-24.f, -32.f});
	player.has_item_equipped(svc.data.item_id_from_label("magnifying_glass")) ? flags.state.set(StateFlags::health_exposed) : flags.state.reset(StateFlags::health_exposed);
	auto flash_rate = 32;
	if (!flags.general.test(GeneralFlags::custom_channels)) {
		set_channel(EnemyChannel::standard);
	} else {
		set_channel(m_custom_channel);
	}
	if (flags.general.test(GeneralFlags::has_invincible_channel)) { flags.state.test(StateFlags::vulnerable) ? set_channel(EnemyChannel::standard) : set_channel(EnemyChannel::invincible); }
	if (hurt_effect.running()) { set_channel((hurt_effect.get() / flash_rate) % 2 == 0 ? EnemyChannel::hurt_1 : EnemyChannel::hurt_2); }

	if (hurt_effect.running() && !flags.state.test(StateFlags::no_shake)) { shake(); }
	hurt_effect.update();

	// shake
	energy = ccm::ext::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { m_random_offset = random::random_vector_float(-energy, energy); }
	if (hitstun.running() && !flags.state.test(StateFlags::no_slowdown)) {
		hitstun.update();
		if (svc.ticker.every_x_ticks(4)) {
		} else {
			return;
		}
	}

	// stuff that slows down from hitstun
	health_indicator.update(svc, get_collider().physics.position);

	if (flags.general.test(GeneralFlags::map_collision)) {
		if (flags.general.test(GeneralFlags::spike_collision)) {
			for (auto& spike : map.spikes) { spike.handle_collision(get_collider()); }
		}
	}

	if (get_collider().collision_depths) { get_collider().collision_depths.value().update(); }
	if (has_secondary_collider()) {
		if (get_secondary_collider().collision_depths) { get_secondary_collider().collision_depths.value().update(); }
		get_secondary_collider().physics.acceleration = {};
	}
	get_collider().physics.acceleration = {};

	on_crush(map);

	if (player.get_collider().wallslider.overlaps(get_collider().bounding_box) && player.controller.is_dashing() && !player.controller.is(player::AbilityType::dash_kick) && !get_collider().has_attribute(shape::ColliderAttributes::sturdy)) {
		if (!player.has_flag_set(player::State::dash_kick)) { hurt(4.f); }
		player.set_flag(player::State::dash_kick);
		get_collider().physics.acceleration.y = -280.f;
	}

	// update ranges
	physical.alert_range.set_position(get_collider().bounding_box.get_position() - (physical.alert_range.get_dimensions() * 0.5f) + (get_collider().dimensions * 0.5f));
	physical.hostile_range.set_position(get_collider().bounding_box.get_position() - (physical.hostile_range.get_dimensions() * 0.5f) + (get_collider().dimensions * 0.5f));
	physical.home_detector.set_position(get_collider().bounding_box.get_position() - (physical.home_detector.get_dimensions() * 0.5f) + (get_collider().dimensions * 0.5f));
	if (player.get_collider().bounding_box.overlaps(physical.alert_range)) {
		if (!is_alert()) { flags.triggers.set(Triggers::alert); }
		flags.state.set(StateFlags::alert);
	} else {
		flags.state.reset(StateFlags::alert);
	}
	if (player.get_collider().bounding_box.overlaps(physical.hostile_range)) {
		if (!is_hostile()) { flags.triggers.set(Triggers::hostile); }
		flags.state.set(StateFlags::hostile);
	} else {
		flags.state.reset(StateFlags::hostile);
	}
}

void Enemy::post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, bool tick) {
	Mobile::post_update(svc, map, player, !flags.general.test(GeneralFlags::no_tick));
	handle_player_collision(player);
}

void Enemy::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (died() && !flags.general.test(GeneralFlags::post_death_render)) { return; }
	auto horizontal_offset = sf::Vector2f{directions.actual.as_float(), 1.f};
	auto sprite_position = get_collider().get_center() - cam + m_random_offset + m_native_offset.componentWiseMul(horizontal_offset);
	Drawable::set_position(sprite_position);

	if (svc.greyblock_mode()) {
		get_collider().render(win, cam);
		if (secondary_collider) { get_secondary_collider().render(win, cam); }
		// physical.alert_range.render(win, cam);
		// physical.hostile_range.render(win, cam);
		// physical.home_detector.render(win, cam, colors::blue);
	}
	// debug();

	Drawable::draw(win);
}

void Enemy::render_indicators(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (flags.state.test(StateFlags::invisible)) { return; }
	if (flags.state.test(StateFlags::health_exposed) && !health.is_dead()) { m_health_bar.render(win, cam); }
	health_indicator.render(svc, win, cam);
}

void Enemy::handle_player_collision(player::Player& player) const {
	if (died()) { return; }
	if (player_collision()) {
		player.get_collider().handle_collider_collision(get_collider());
		if (has_secondary_collider()) { player.get_collider().handle_collider_collision(get_secondary_collider()); }
	}
	if (flags.general.test(GeneralFlags::hurt_on_contact)) {
		if (player.get_collider().hurtbox.overlaps(get_collider().bounding_box)) { player.hurt(attributes.base_damage); }
	}
}

void Enemy::handle_collision(shape::Collider& other) { get_collider().handle_collider_collision(other, true); }

void Enemy::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.get_team() == arms::Team::skycorps) { return; }
	if (proj.get_team() == arms::Team::guardian) { return; }
	if (proj.get_team() == arms::Team::beast) { return; }
	if (flags.state.test(StateFlags::intangible)) { return; }
	if (flags.state.test(StateFlags::invisible)) { return; }
	if (health.is_dead()) { return; }
	auto hit_main = proj.get_collider().collides_with(get_collider().bounding_box);
	auto hit_second = secondary_collider ? proj.get_collider().collides_with(get_secondary_collider().bounding_box) : false;
	if (!(hit_main || hit_second)) { return; }
	flags.state.set(enemy::StateFlags::shot);
	auto secondary_collision = hit_second && !hit_main;
	if (((secondary_collision && flags.general.test(GeneralFlags::invincible_secondary)) || !flags.state.test(enemy::StateFlags::vulnerable)) && !died()) {
		map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
		svc.soundboard.flags.world.set(audio::World::hard_hit);
	} else if (flags.state.test(enemy::StateFlags::vulnerable) && !died()) {
		if (proj.persistent()) { proj.damage_over_time(); }
		if (proj.can_damage()) {
			hurt(proj.get_damage());
			if (!flags.general.test(GeneralFlags::custom_sounds) && !sound.hurt_sound_cooldown.running()) { svc.soundboard.flags.enemy.set(sound.hit_flag); }
			map.effects.push_back(entity::Effect(svc, "hit_flash", proj.get_position()));
			hitstun.start(32);
		}
		svc.soundboard.flags.world.set(audio::World::projectile_hit);
	}
	if (!proj.persistent() && (!died() || just_died())) { proj.destroy(false); }
}

void Enemy::spawn_treasure(automa::ServiceProvider& svc, world::Map& map) {
	if (!m_treasure) { return; }
	if (m_treasure->empty()) { return; }
	Treasure const& chosen = random::weightedChoice(*m_treasure, [](Treasure const& t) { return t.drop_chance; });
	map.spawn_chest(svc, chosen, get_collider().get_center(), random::random_vector_float({-1.f, 1.f}, {-12.f, -20.f}));
}

void Enemy::hurt(float amount) {
	flags.state.set(StateFlags::hurt);
	health.inflict(amount);
	health_indicator.add(-amount);
}

void Enemy::on_crush(world::Map& map) {
	if (!get_collider().collision_depths) { return; }
	if (flags.general.test(GeneralFlags::uncrushable)) { return; }
	auto second_crush = secondary_collider ? get_secondary_collider().crushed() : false;
	if (get_collider().crushed() || second_crush) {
		hurt(1024.f);
		get_collider().collision_depths = {};
	}
}

bool Enemy::seek_home(world::Map& map) {
	auto distance = std::numeric_limits<float>::max();
	auto my_point = sf::Vector2f{};
	for (auto home : map.home_points) { distance = std::min(distance, (home - physical.home_detector.get_center()).length()); }
	for (auto home : map.home_points) {
		if ((home - physical.home_detector.get_center()).length() <= distance && distance < 2048.f) { my_point = home; }
	}

	// my_point is our target
	flags.state.reset(StateFlags::advance);
	if (my_point.length() > 0.1f) {
		flags.state.set(StateFlags::advance);
		directions.desired.set((my_point.x < get_collider().get_center().x) ? LNR::left : LNR::right);
	}
	if (physical.home_detector.overlaps(my_point)) {
		flags.state.reset(StateFlags::advance);
		return true;
	}
	return false;
}

void Enemy::set_position_from_scaled(sf::Vector2f pos) {
	auto new_pos = pos;
	auto round = static_cast<int>(get_collider().dimensions.y) % 32;
	new_pos.y += static_cast<float>(32.f - round);
	set_position(new_pos);
}

void Enemy::debug() {
	static bool* b_debug{};
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	static ImVec2 pos = ImVec2{Drawable::get_window_position()};
	ImGui::SetNextWindowPos(ImVec2{0, 0}, ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.65f);
	if (ImGui::Begin("Enemy Info", b_debug, window_flags)) {
		ImGui::Separator();
		ImGui::Text("Label: %s", label.c_str());
		ImGui::Text("Desired Direction: %s", directions.desired.print_lnr().c_str());
		ImGui::SameLine();
		ImGui::Text("%s", directions.desired.is_locked() ? "(locked)" : "");
		ImGui::Text("Actual Direction: %s", directions.actual.print_lnr().c_str());
	}
	ImGui::End();
}

} // namespace fornani::enemy
