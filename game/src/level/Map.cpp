#include "fornani/level/Map.hpp"
#include <imgui.h>
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Math.hpp"

#include <tracy/Tracy.hpp>

#include <iostream>

#include "fornani/utils/Random.hpp"

namespace fornani::world {

Map::Map(automa::ServiceProvider& svc, player::Player& player, gui::Console& console)
	: player(&player), enemy_catalog(svc), save_point(svc), transition(svc, 96), soft_reset(svc, 64), m_services(&svc), m_console(&console), cooldowns{.fade_obscured{128}, .loading{2}}, barrier{1.f, 1.f},
	  scaled_barrier{barrier * svc.constants.cell_size} {}

void Map::load(automa::ServiceProvider& svc, int room_number, bool soft) {

	// for debugging
	center_box.setSize(svc.constants.f_screen_dimensions * 0.5f);
	flags.state.reset(LevelState::game_over);
	if (!player->is_dead()) { svc.state_controller.actions.reset(automa::Actions::death_mode); }
	spawn_counter.start();

	svc.state_flags.reset(automa::StateFlags::hide_hud);

	int ctr{};
	for (auto& room : svc.data.map_jsons) {
		if (room.id == room_number) { room_lookup = ctr; }
		++ctr;
	}
	auto const& metadata = svc.data.map_jsons.at(room_lookup).metadata;
	inspectable_data = svc.data.map_jsons.at(room_lookup).metadata["entities"]["inspectables"];

	// check for enemy respawns
	svc.data.respawn_enemies(room_id, player->visit_history.distance_traveled_from(room_id));

	auto const& meta = metadata["meta"];
	room_id = meta["room_id"].as<int>();
	if (meta["minimap"].as_bool()) { flags.properties.set(MapProperties::minimap); }
	metagrid_coordinates.x = meta["metagrid"][0].as<int>();
	metagrid_coordinates.y = meta["metagrid"][1].as<int>();
	dimensions.x = meta["dimensions"][0].as<int>();
	dimensions.y = meta["dimensions"][1].as<int>();
	real_dimensions = {static_cast<float>(dimensions.x) * svc.constants.cell_size, static_cast<float>(dimensions.y) * svc.constants.cell_size};
	auto style_value = meta["style"].as<int>();
	style_label = svc.data.map_styles["styles"][style_value]["label"].as_string();
	style_id = svc.data.map_styles["styles"][style_value]["id"].as<int>();
	if (svc.greyblock_mode()) { style_id = static_cast<int>(lookup::Style::provisional); }
	native_style_id = svc.data.map_styles["styles"][style_value]["id"].as<int>();
	m_middleground = svc.data.map_jsons.at(room_lookup).metadata["tile"]["middleground"].as<int>();
	svc.data.map_jsons.at(room_lookup).metadata["tile"]["flags"]["obscuring"].as_bool() ? flags.properties.set(MapProperties::has_obscuring_layer) : flags.properties.reset(MapProperties::has_obscuring_layer);
	svc.data.map_jsons.at(room_lookup).metadata["tile"]["flags"]["reverse_obscuring"].as_bool() ? flags.properties.set(MapProperties::has_reverse_obscuring_layer) : flags.properties.reset(MapProperties::has_reverse_obscuring_layer);

	auto const& entities = metadata["entities"];

	if (!soft) {
		if (meta["cutscene_on_entry"]["flag"].as_bool()) {
			auto ctype = meta["cutscene_on_entry"]["type"].as<int>();
			auto cid = meta["cutscene_on_entry"]["id"].as<int>();
			auto csource = meta["cutscene_on_entry"]["source"].as<int>();
			auto cutscene = util::QuestKey{ctype, cid, csource};
			svc.quest.process(svc, cutscene);
			if (svc.quest.get_progression(fornani::QuestType::cutscene, 6001) > 0) { std::cout << "cutscene requested!\n"; }
			cutscene_catalog.push_cutscene(svc, *this, *m_console, cid);
		}
		if (meta["music"].is_string()) {
			svc.music.load(svc.finder, meta["music"].as_string());
			svc.music.play_looped(10);
		}
		if (meta["ambience"].is_string()) {
			ambience.load(svc.finder, meta["ambience"].as_string());
			ambience.play();
		}
		for (auto& entry : meta["atmosphere"].array_view()) {
			if (entry.as<int>() == 1) { atmosphere.push_back(vfx::Atmosphere(svc, real_dimensions, 1)); }
		}
		if (meta["camera_effects"]) {
			m_camera_effects.shake_properties.frequency = meta["camera_effects"]["shake"]["frequency"].as<int>();
			m_camera_effects.shake_properties.energy = meta["camera_effects"]["shake"]["energy"].as<float>();
			m_camera_effects.shake_properties.start_time = meta["camera_effects"]["shake"]["start_time"].as<float>();
			m_camera_effects.shake_properties.dampen_factor = meta["camera_effects"]["shake"]["dampen_factor"].as<int>();
			m_camera_effects.cooldown = util::Cooldown{meta["camera_effects"]["shake"]["frequency_in_seconds"].as<int>()};
			m_camera_effects.shake_properties.shaking = m_camera_effects.shake_properties.frequency > 0;
		}

		sound.echo_count = meta["sound"]["echo_count"].as<int>();
		sound.echo_rate = meta["sound"]["echo_rate"].as<int>();

		if (meta["weather"]["rain"]) { rain = vfx::Rain(meta["weather"]["rain"]["intensity"].as<int>(), meta["weather"]["rain"]["fall_speed"].as<float>(), meta["weather"]["rain"]["slant"].as<float>()); }
		if (meta["weather"]["snow"]) { rain = vfx::Rain(meta["weather"]["snow"]["intensity"].as<int>(), meta["weather"]["snow"]["fall_speed"].as<float>(), meta["weather"]["snow"]["slant"].as<float>(), true); }
		if (meta["weather"]["leaves"]) { rain = vfx::Rain(meta["weather"]["leaves"]["intensity"].as<int>(), meta["weather"]["leaves"]["fall_speed"].as<float>(), meta["weather"]["leaves"]["slant"].as<float>(), true, true); }

		background = std::make_unique<bg::Background>(svc, meta["background"].as<int>());
		styles.breakables = meta["styles"]["breakables"].as<int>();
		styles.pushables = meta["styles"]["pushables"].as<int>();

		for (auto& entry : entities["npcs"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>();
			pos.y = entry["position"][1].as<float>();
			auto id = entry["id"].as<int>();
			npcs.push_back(npc::NPC(svc, id));
			auto npc_state = svc.quest.get_progression(fornani::QuestType::npc, id);
			for (auto& convo : entry["suites"][npc_state].array_view()) { npcs.back().push_conversation(convo.as_string()); }
			npcs.back().set_position_from_scaled(pos);
			if ((bool)entry["background"].as_bool()) { npcs.back().push_to_background(); }
			if (static_cast<bool>(entry["hidden"].as_bool())) { npcs.back().hide(); }
			if (svc.quest.get_progression(fornani::QuestType::hidden_npcs, id) > 0) { npcs.back().unhide(); }
			npcs.back().set_current_location(room_id);
		}
		for (auto& entry : entities["chests"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>();
			pos.y = entry["position"][1].as<float>();
			chests.push_back(entity::Chest(svc, entry["id"].as<int>()));
			chests.back().set_item(entry["item_id"].as<int>());
			chests.back().set_amount(entry["amount"].as<int>());
			chests.back().set_rarity(entry["rarity"].as<float>());
			if (entry["type"].as<int>() == 1) { chests.back().set_type(entity::ChestType::gun); }
			if (entry["type"].as<int>() == 2) { chests.back().set_type(entity::ChestType::orbs); }
			if (entry["type"].as<int>() == 3) { chests.back().set_type(entity::ChestType::item); }
			chests.back().set_position_from_scaled(pos);
		}

		for (auto& entry : entities["animators"].array_view()) {
			sf::Vector2<int> scaled_dim{};
			sf::Vector2<int> scaled_pos{};
			scaled_pos.x = entry["position"][0].as<int>();
			scaled_pos.y = entry["position"][1].as<int>();
			scaled_dim.x = entry["dimensions"][0].as<int>();
			scaled_dim.y = entry["dimensions"][1].as<int>();
			auto automatic = (bool)entry["automatic"].as_bool();
			auto astyle = (bool)entry["style"].as<int>();
			auto lg = scaled_dim.x == 2;
			auto foreground = (bool)entry["foreground"].as_bool();
			auto aid = entry["id"].as<int>();
			auto a = entity::Animator(svc, scaled_pos, aid, lg, automatic, foreground, astyle);
			animators.push_back(a);
		}
		for (auto& entry : entities["beds"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>() * svc.constants.cell_size;
			pos.y = entry["position"][1].as<float>() * svc.constants.cell_size;
			beds.push_back(entity::Bed(svc, pos, room_lookup));
		}
		for (auto& entry : entities["scenery"]["basic"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>() * svc.constants.cell_size;
			pos.y = entry["position"][1].as<float>() * svc.constants.cell_size;
			auto var = entry["variant"].as<int>();
			auto lyr = entry["layer"].as<int>();
			auto parallax = entry["parallax"].as<float>();
			scenery_layers.at(lyr).push_back(std::make_unique<vfx::Scenery>(svc, pos, style_id, lyr, var, parallax));
		}
		for (auto& entry : entities["scenery"]["vines"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>() * svc.constants.cell_size;
			pos.y = entry["position"][1].as<float>() * svc.constants.cell_size;
			auto fg = static_cast<bool>(entry["foreground"].as_bool());
			auto rev = static_cast<bool>(entry["reversed"].as_bool());
			vines.push_back(std::make_unique<entity::Vine>(svc, pos, entry["length"].as<int>(), entry["size"].as<int>(), fg, rev));
			if (entry["platform"]) {
				for (auto& link : entry["platform"]["link_indeces"].array_view()) { vines.back()->add_platform(svc, link.as<int>()); }
			}
		}
		for (auto& entry : entities["scenery"]["grass"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>() * svc.constants.cell_size;
			pos.y = entry["position"][1].as<float>() * svc.constants.cell_size;
			auto fg = static_cast<bool>(entry["foreground"].as_bool());
			grass.push_back(std::make_unique<entity::Grass>(svc, pos, 8, entry["size"].as<int>(), fg));
		}
		for (auto& entry : entities["inspectables"].array_view()) {
			sf::Vector2<uint32_t> dim{};
			sf::Vector2<uint32_t> pos{};
			auto key = entry["key"].as_string();
			pos.x = entry["position"][0].as<int>();
			pos.y = entry["position"][1].as<int>();
			dim.x = entry["dimensions"][0].as<int>();
			dim.y = entry["dimensions"][1].as<int>();
			auto alt = entry["alternates"].as<int>();
			auto native = entry["native_id"].as<int>();
			auto aoc = static_cast<bool>(entry["activate_on_contact"].as_bool());
			inspectables.push_back(entity::Inspectable(svc, dim, pos, key, room_id, alt, native, aoc));
			if (svc.data.inspectable_is_destroyed(inspectables.back().get_id())) { inspectables.back().destroy(); }
		}

		for (auto& entry : entities["enemies"].array_view()) {
			int id{};
			sf::Vector2<float> pos{};
			sf::Vector2<int> start{};
			pos.x = entry["position"][0].as<float>();
			pos.y = entry["position"][1].as<float>();
			start.x = entry["start_direction"][0].as<int>();
			start.y = entry["start_direction"][1].as<int>();
			auto variant = entry["variant"].as<int>();
			enemy_catalog.push_enemy(svc, *this, *m_console, entry["id"].as<int>(), false, variant, start);
			enemy_catalog.enemies.back()->set_position_from_scaled({pos * svc.constants.cell_size});
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			enemy_catalog.enemies.back()->set_external_id({room_id, {static_cast<int>(pos.x), static_cast<int>(pos.y)}});
			std::cout << "Enemy external id: " << enemy_catalog.enemies.back()->get_external_id() << "\n";
			if (svc.data.enemy_is_fallen(room_id, enemy_catalog.enemies.back()->get_external_id())) { enemy_catalog.enemies.pop_back(); }
		}
		for (auto& entry : entities["destroyers"].array_view()) {
			sf::Vector2<int> pos{};
			pos.x = entry["position"][0].as<int>();
			pos.y = entry["position"][1].as<int>();
			auto quest_id = entry["quest_id"].as<int>();
			destroyers.push_back(Destroyable(svc, pos, quest_id));
		}
	}

	for (auto& entry : entities["portals"].array_view()) {
		sf::Vector2<uint32_t> pos{};
		sf::Vector2<uint32_t> dim{};
		pos.x = entry["position"][0].as<int>();
		pos.y = entry["position"][1].as<int>();
		dim.x = entry["dimensions"][0].as<int>();
		dim.y = entry["dimensions"][1].as<int>();
		auto src_id = entry["source_id"].as<int>();
		auto dest_id = entry["destination_id"].as<int>();
		auto aoc = static_cast<bool>(entry["activate_on_contact"].as_bool());
		auto locked = static_cast<bool>(entry["locked"].as_bool());
		auto already_open = static_cast<bool>(entry["already_open"].as_bool());
		auto key_id = entry["key_id"].as<int>();
		auto door_style = native_style_id;
		auto mapdim = sf::Vector2<int>{dimensions};
		portals.push_back(entity::Portal(svc, dim, pos, src_id, dest_id, aoc, locked, already_open, key_id, door_style, mapdim));
		portals.back().update(svc);
	}

	auto const& savept = entities["save_point"];
	auto save_id = svc.state_controller.save_point_id;
	save_point.id = savept.contains("position") ? room_id : -1;
	save_point.scaled_position.x = savept["position"][0].as<int>();
	save_point.scaled_position.y = savept["position"][1].as<int>();

	for (auto& entry : entities["platforms"].array_view()) {
		sf::Vector2<float> dim{};
		sf::Vector2<float> pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		dim.x = entry["dimensions"][0].as<float>();
		dim.y = entry["dimensions"][1].as<float>();
		pos *= svc.constants.cell_size;
		dim *= svc.constants.cell_size;
		auto start = entry["start"].as<float>();
		start = std::clamp(start, 0.f, 1.f);
		auto type = entry["type"].as_string();
		platforms.push_back(Platform(svc, pos, dim, entry["extent"].as<float>(), type, start, entry["style"].as<int>()));
	}
	for (auto& entry : entities["switch_blocks"].array_view()) {
		sf::Vector2<float> pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		pos *= svc.constants.cell_size;
		auto type = entry["type"].as<int>();
		auto button_id = entry["button_id"].as<int>();
		switch_blocks.push_back(SwitchBlock(svc, pos, button_id, type));
	}
	for (auto& entry : entities["switches"].array_view()) {
		sf::Vector2<float> pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		pos *= svc.constants.cell_size;
		auto type = entry["type"].as<int>();
		auto button_id = entry["button_id"].as<int>();
		switch_buttons.push_back(std::make_unique<SwitchButton>(svc, pos, button_id, type, *this));
	}

	generate_collidable_layer();

	if (!soft) {
		generate_layer_textures(svc);
		player->map_reset();
		transition.end();
		cooldowns.fade_obscured.start();
		cooldowns.loading.start();
	}
}

void Map::update(automa::ServiceProvider& svc, gui::Console& console, gui::InventoryWindow& inventory_window) {
	auto& layers = svc.data.get_layers(room_id);
	flags.state.reset(LevelState::camera_shake);
	cooldowns.loading.update();

	// camera effects
	if (svc.ticker.every_second() && m_camera_effects.shake_properties.shaking) {
		m_camera_effects.cooldown.update();
		if (m_camera_effects.cooldown.is_complete()) {
			svc.camera_controller.shake(m_camera_effects.shake_properties);
			auto shake_time = m_camera_effects.cooldown.get_native_time();
			auto diff = util::Random::random_range(-shake_time / 2, shake_time / 2);
			m_camera_effects.cooldown.start(shake_time + diff);
		}
	}

	for (auto& cutscene : cutscene_catalog.cutscenes) { cutscene->update(svc, console, *this, *player); }

	if (flags.state.test(LevelState::spawn_enemy)) {
		for (auto& spawn : enemy_spawns) {
			enemy_catalog.push_enemy(*m_services, *this, *m_console, spawn.id, true);
			enemy_catalog.enemies.back()->set_position(spawn.pos);
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			effects.push_back(entity::Effect(*m_services, spawn.pos + enemy_catalog.enemies.back()->get_collider().dimensions * 0.5f, {}, 0, 4));
		}
		enemy_spawns.clear();
		flags.state.reset(LevelState::spawn_enemy);
	}

	inventory_window.update(svc, *player, *this);

	player->collider.reset();
	for (auto& a : player->antennae) { a.collider.reset(); }
	if (off_the_bottom(player->collider.physics.position) && cooldowns.loading.is_complete()) {
		player->hurt(64.f);
		player->freeze_position();
	}

	// hidden areas
	flags.map_state.test(MapState::unobscure) ? cooldowns.fade_obscured.update() : cooldowns.fade_obscured.reverse();
	if (check_cell_collision(player->collider, true)) {
		if (!flags.map_state.test(MapState::unobscure)) { cooldowns.fade_obscured.start(); }
		flags.map_state.set(MapState::unobscure);
	} else {
		if (flags.map_state.test(MapState::unobscure)) { cooldowns.fade_obscured.cancel(); }
		flags.map_state.reset(MapState::unobscure);
	}

	for (auto& grenade : active_grenades) {
		if (player->shielding() && player->controller.get_shield().sensor.within_bounds(grenade.bounding_box)) {
			player->controller.get_shield().damage();
			grenade.physics.velocity *= -1.f;
		}
		if (grenade.detonated() && grenade.sensor.within_bounds(player->hurtbox)) { player->hurt(grenade.get_damage()); }
		for (auto& enemy : enemy_catalog.enemies) {
			if (grenade.detonated() && grenade.sensor.within_bounds(enemy->get_collider().hurtbox)) {
				enemy->hurt();
				enemy->health.inflict(grenade.get_damage());
				enemy->health_indicator.add(grenade.get_damage());
				if (enemy->just_died() && enemy->spawn_loot()) {
					svc.stats.enemy.enemies_killed.update();
					active_loot.push_back(item::Loot(svc, enemy->get_attributes().drop_range, enemy->get_attributes().loot_multiplier, enemy->get_collider().bounding_box.get_position()));
					svc.soundboard.flags.frdog.set(audio::Frdog::death);
				}
			}
		}
	}

	for (auto& grenade : active_grenades) {
		for (auto& breakable : breakables) {
			if (grenade.detonated() && grenade.sensor.within_bounds(breakable.get_bounding_box())) { breakable.destroy(); }
		}
	}

	// TODO: refactor this
	if (svc.player_dat.piggy_id != 0) {
		for (auto& n : npcs) {
			if (n.get_id() == svc.player_dat.piggy_id) { n.hide(); }
		}
	}
	if (svc.player_dat.drop_piggy) {
		svc.player_dat.drop_piggy = false;
		for (auto& n : npcs) {
			if (n.get_id() == svc.player_dat.piggy_id) {
				n.unhide();
				n.set_position(player->collider.physics.position);
				n.apply_force({32.f, -32.f});
				n.set_current_location(room_id);
			}
		}
		svc.player_dat.piggy_id = 0;
	}

	std::erase_if(active_emitters, [](auto const& p) { return p.done(); });
	std::erase_if(effects, [](auto& e) { return e.done(); });
	std::erase_if(active_grenades, [](auto const& g) { return g.detonated(); });
	std::erase_if(breakables, [](auto const& b) { return b.destroyed(); });
	std::erase_if(inspectables, [](auto const& i) { return i.destroyed(); });
	std::erase_if(destroyers, [](auto const& d) { return d.detonated(); });
	std::erase_if(npcs, [](auto const& n) { return n.piggybacking(); });

	manage_projectiles(svc);
	svc.map_debug.active_projectiles = active_projectiles.size();
	for (auto& proj : active_projectiles) {
		if (proj.destruction_initiated()) { continue; }
		for (auto& platform : platforms) { platform.on_hit(svc, *this, proj); }
		for (auto& breakable : breakables) { breakable.on_hit(svc, *this, proj); }
		for (auto& pushable : pushables) { pushable.on_hit(svc, *this, proj); }
		for (auto& destroyer : destroyers) { destroyer.on_hit(svc, *this, proj); }
		for (auto& block : switch_blocks) { block.on_hit(svc, *this, proj); }
		for (auto& enemy : enemy_catalog.enemies) { enemy->on_hit(svc, *this, proj); }
		for (auto& vine : vines) { vine->on_hit(svc, *this, proj); }
		proj.handle_collision(svc, *this);
		proj.on_player_hit(*player);
	}
	for (auto& enemy : enemy_catalog.enemies) {
		enemy->unique_update(svc, *this, *player);
		enemy->post_update(svc, *this, *player);
	}

	if (fire) {
		for (auto& f : fire.value()) { f.update(svc, *player, *this, console, inspectable_data); }
	}
	for (auto& loot : active_loot) { loot.update(svc, *this, *player); }
	for (auto& grenade : active_grenades) { grenade.update(svc, *player, *this); }
	for (auto& emitter : active_emitters) { emitter.update(svc, *this); }
	for (auto& chest : chests) { chest.update(svc, *this, console, *player); }
	for (auto& npc : npcs) { npc.update(svc, *this, console, *player); }
	for (auto& portal : portals) { portal.handle_activation(svc, *player, console, room_id, transition); }
	for (auto& inspectable : inspectables) { inspectable.update(svc, *player, console, svc.data.map_jsons.at(room_lookup).metadata["entities"]["inspectables"]); }
	for (auto& animator : animators) { animator.update(svc, *player); }
	for (auto& effect : effects) { effect.update(svc, *this); }
	for (auto& atm : atmosphere) { atm.update(svc, *this, *player); }
	for (auto& platform : platforms) { platform.update(svc, *this, *player); }
	for (auto& spawner : spawners) { spawner.update(svc, *this); }
	for (auto& switch_block : switch_blocks) { switch_block.update(svc, *this, *player); }
	for (auto& switch_button : switch_buttons) { switch_button->update(svc, *this, *player); }
	for (auto& destroyer : destroyers) { destroyer.update(svc, *this, *player); }
	for (auto& checkpoint : checkpoints) { checkpoint.update(svc, *this, *player); }
	for (auto& bed : beds) { bed.update(svc, *this, console, *player, transition); }
	for (auto& breakable : breakables) { breakable.update(svc, *player); }
	for (auto& pushable : pushables) { pushable.update(svc, *this, *player); }
	for (auto& spike : spikes) { spike.update(svc, *player, *this); }
	for (auto& vine : vines) { vine->update(svc, *this, *player); }
	for (auto& g : grass) { g->update(svc, *this, *player); }
	player->handle_map_collision(*this);
	if (cooldowns.loading.is_complete()) { transition.update(*player); }
	soft_reset.update(*player);
	if (player->collider.collision_depths) { player->collider.collision_depths.value().update(); }
	if (save_point.id != -1) { save_point.update(svc, *player, console); }
	if (rain) { rain.value().update(svc, *this); }

	console.update(svc);

	player->collider.reset_ground_flags();

	// ambience
	ambience.set_balance(cooldowns.fade_obscured.get_normalized() * 100.f);

	// check if player died
	if (!flags.state.test(LevelState::game_over) && player->death_animation_over() && svc.death_mode() && cooldowns.loading.is_complete()) {
		svc.app_flags.reset(automa::AppFlags::in_game);
		console.set_source(svc.text.basic);
		console.load_and_launch("death");
		flags.state.set(LevelState::game_over);
		svc.music.load(svc.finder, "mortem");
		svc.music.play_looped(10);
		svc.soundboard.turn_off();
		svc.stats.player.death_count.update();
	}

	// demo only
	if (svc.state_controller.actions.consume(automa::Actions::end_demo)) { end_demo.start(); }
	end_demo.update();
	if (end_demo.get_cooldown() == 1) {
		m_console->set_source(svc.text.basic);
		m_console->load_and_launch("end_demo");
	}
	if (svc.state_controller.actions.test(automa::Actions::print_stats) && console.is_complete()) { svc.state_controller.actions.set(automa::Actions::trigger); }
	// demo only

	if (svc.state_controller.actions.test(automa::Actions::retry)) { flags.state.set(LevelState::game_over); }
	if (console.is_complete() && flags.state.test(LevelState::game_over)) {
		transition.start();
		if (transition.is_done()) {
			player->start_over();
			svc.state_controller.actions.set(automa::Actions::player_death);
			svc.state_controller.actions.set(automa::Actions::trigger);
		}
	}

	console.clean_off_trigger();
	inventory_window.clean_off_trigger();
	inventory_window.info.clean_off_trigger();
}

void Map::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	ZoneScopedN("Map::render");
	// check for a switch to greyblock mode
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_trigger)) {
		style_id = style_id == static_cast<int>(lookup::Style::provisional) ? native_style_id : static_cast<int>(lookup::Style::provisional);
		generate_layer_textures(svc);
		svc.debug_flags.reset(automa::DebugFlags::greyblock_trigger);
	}

	{
		ZoneScopedN("Map::render - portals");
		for (auto& portal : portals) { portal.render(svc, win, cam); }
	}
	if (fire) {
		ZoneScopedN("Map::render - fires");
		for (auto& f : fire.value()) { f.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - chests");
		for (auto& chest : chests) { chest.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - npcs");
		for (auto& npc : npcs) {
			if (!npc.background()) { npc.render(svc, win, cam); }
		}
	}
	{
		ZoneScopedN("Map::render - active grenades");
		for (auto& grenade : active_grenades) { grenade.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - player");
		player->render(svc, win, cam);
	}
	{
		ZoneScopedN("Map::render - enemies");
		for (auto& enemy : enemy_catalog.enemies) {
			if (!enemy->is_foreground()) {
				enemy->render(svc, win, cam);
				enemy->unique_render(svc, win, cam);
			}
		}
	}
	{
		ZoneScopedN("Map::render - active projectiles");
		for (auto& proj : active_projectiles) { proj.render(svc, *player, win, cam); }
	}
	{
		ZoneScopedN("Map::render - active loot");
		for (auto& loot : active_loot) { loot.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - active emmitters");
		for (auto& emitter : active_emitters) { emitter.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - platforms");
		for (auto& platform : platforms) { platform.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - breakables");
		for (auto& breakable : breakables) { breakable.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - pushables");
		for (auto& pushable : pushables) { pushable.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - destroyers");
		for (auto& destroyer : destroyers) { destroyer.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - checkpoints");
		for (auto& checkpoint : checkpoints) { checkpoint.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - spikes");
		for (auto& spike : spikes) { spike.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - switch blocks");
		for (auto& switch_block : switch_blocks) { switch_block.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - switch buttons");
		for (auto& switch_button : switch_buttons) { switch_button->render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - beds");
		for (auto& bed : beds) { bed.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - atmosphere");
		for (auto& atm : atmosphere) { atm.render(svc, win, cam); }
	}
	{
		ZoneScopedN("Map::render - vines");
		for (auto& vine : vines) {
			if (vine->foreground()) { vine->render(svc, win, cam); }
		}
	}
	{
		ZoneScopedN("Map::render - grass");
		for (auto& g : grass) {
			if (g->foreground()) { g->render(svc, win, cam); }
		}
	}

	if (save_point.id != -1) {
		ZoneScopedN("Map::render - save point");
		save_point.render(svc, win, cam);
	}

	// map foreground tiles
	if (!svc.greyblock_mode()) {
		std::vector<sf::Sprite> sprites{sf::Sprite{textures.foreground.day.getTexture()}, sf::Sprite{textures.foreground.twilight.getTexture()}, sf::Sprite{textures.foreground.night.getTexture()}};
		auto ctr{0};
		for (auto& sprite : sprites) {
			sprite.setPosition(-cam - scaled_barrier);
			m_camera_effects.shifter.render(svc, win, sprite, ctr);
			++ctr;
		}
	}

	{
		ZoneScopedN("Map::render - forground enemies");
		// foreground enemies
		for (auto& enemy : enemy_catalog.enemies) {
			if (enemy->is_foreground()) {
				enemy->render(svc, win, cam);
				enemy->unique_render(svc, win, cam);
			}
			enemy->render_indicators(svc, win, cam);
			enemy->gui_render(svc, win, cam);
		}
	}

	if (!svc.greyblock_mode()) {
		// draw obscuring layer
		if (textures.obscuring) {
			std::vector<sf::Sprite> obs_sprites{sf::Sprite{textures.obscuring.value().day.getTexture()}, sf::Sprite{textures.obscuring.value().twilight.getTexture()}, sf::Sprite{textures.obscuring.value().night.getTexture()}};

			auto ctr{0};
			for (auto& sprite : obs_sprites) {
				sprite.setPosition(-cam - scaled_barrier);
				std::uint8_t alpha = std::lerp(0, 255, cooldowns.fade_obscured.get_normalized());
				if (alpha != 0) { m_camera_effects.shifter.render(svc, win, sprite, ctr, alpha); }
				++ctr;
			}
		}
		if (textures.reverse_obscuring) {
			std::vector<sf::Sprite> rev_sprites{sf::Sprite{textures.reverse_obscuring.value().day.getTexture()}, sf::Sprite{textures.reverse_obscuring.value().twilight.getTexture()},
												sf::Sprite{textures.reverse_obscuring.value().night.getTexture()}};

			auto ctr{0};
			for (auto& sprite : rev_sprites) {
				sprite.setPosition(-cam - scaled_barrier);
				std::uint8_t revalpha = std::lerp(0, 255, 1.f - cooldowns.fade_obscured.get_normalized());
				if (revalpha != 0) { m_camera_effects.shifter.render(svc, win, sprite, ctr, revalpha); }
				++ctr;
			}
		}
	}

	{
		ZoneScopedN("Map::render - effects");
		for (auto& effect : effects) { effect.render(svc, win, cam); }
	}

	player->render_indicators(svc, win, cam);

	if (real_dimensions.y < svc.constants.f_screen_dimensions.y) {
		auto ydiff = (svc.constants.f_screen_dimensions.y - real_dimensions.y) * 0.5f;
		borderbox.setFillColor(svc.styles.colors.ui_black);
		borderbox.setSize({svc.constants.f_screen_dimensions.x, ydiff});
		borderbox.setPosition({});
		win.draw(borderbox);

		borderbox.setPosition({0.0f, real_dimensions.y + ydiff});
		win.draw(borderbox);
	}
	if (real_dimensions.x < svc.constants.f_screen_dimensions.x) {
		auto xdiff = (svc.constants.f_screen_dimensions.x - real_dimensions.x) * 0.5f;
		borderbox.setFillColor(svc.styles.colors.ui_black);
		borderbox.setSize({xdiff, svc.constants.f_screen_dimensions.y});
		borderbox.setPosition({});
		win.draw(borderbox);

		borderbox.setPosition({real_dimensions.x + xdiff, 0.0f});
		win.draw(borderbox);
	}

	{
		ZoneScopedN("Map::render - animator");
		for (auto& animator : animators) {
			if (animator.foreground()) { animator.render(svc, win, cam); }
		}
	}
	{
		ZoneScopedN("Map::render - inspectables");
		for (auto& inspectable : inspectables) { inspectable.render(svc, win, cam); }
	}
	if (rain) {
		ZoneScopedN("Map::render - rain");
		rain.value().render(svc, win, cam);
	}

	if (svc.greyblock_mode()) {
		center_box.setPosition({});
		center_box.setFillColor(sf::Color(80, 80, 80, 60));
		win.draw(center_box);
		center_box.setPosition(svc.constants.f_screen_dimensions * 0.5f);
		win.draw(center_box);
		center_box.setFillColor(sf::Color(100, 100, 100, 60));
		center_box.setPosition({svc.constants.f_screen_dimensions.x * 0.5f, 0.f});
		win.draw(center_box);
		center_box.setPosition({0.f, svc.constants.f_screen_dimensions.y * 0.5f});
		win.draw(center_box);
		sf::Sprite greyblock{textures.greyblock.getTexture()};
		greyblock.setPosition(-cam);
		win.draw(greyblock);
		get_middleground().grid.render(win, cam);
	}
}

void Map::render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	ZoneScopedN("Map::render_background");
	if (!svc.greyblock_mode()) {
		background->render(svc, win, cam);
		for (auto& layer : scenery_layers) {
			for (auto& piece : layer) { piece->render(svc, win, cam); }
		}
		std::vector<sf::Sprite> sprites{sf::Sprite{textures.background.day.getTexture()}, sf::Sprite{textures.background.twilight.getTexture()}, sf::Sprite{textures.background.night.getTexture()}};
		auto ctr{0};
		for (auto& sprite : sprites) {
			sprite.setPosition(-cam - scaled_barrier);
			m_camera_effects.shifter.render(svc, win, sprite, ctr);
			++ctr;
		}
		for (auto& npc : npcs) {
			if (npc.background()) { npc.render(svc, win, cam); }
		}
		for (auto& switch_block : switch_blocks) { switch_block.render(svc, win, cam, true); }
	} else {
		sf::RectangleShape box{};
		box.setFillColor(svc.styles.colors.black);
		box.setSize(svc.constants.f_screen_dimensions);
		win.draw(box);
	}

	for (auto& vine : vines) {
		if (!vine->foreground()) { vine->render(svc, win, cam); }
	}
	for (auto& g : grass) {
		if (!g->foreground()) { g->render(svc, win, cam); }
	}
	for (auto& animator : animators) {
		if (!animator.foreground()) { animator.render(svc, win, cam); }
	}
}

void Map::render_console(automa::ServiceProvider& svc, gui::Console& console, sf::RenderWindow& win) {
	ZoneScopedN("Map::render_console");
	if (console.flags.test(gui::ConsoleFlags::active)) { console.render(win); }
	console.write(win, false);
}

void Map::spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2<float> pos, sf::Vector2<float> target) {
	active_projectiles.push_back(weapon.projectile);
	active_projectiles.back().set_position(pos);
	active_projectiles.back().seed(svc, target);
	active_projectiles.back().update(svc, *player);

	if (weapon.secondary_emitter) {
		active_emitters.push_back(vfx::Emitter(svc, weapon.get_barrel_point(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().type, weapon.secondary_emitter.value().color, weapon.get_firing_direction()));
	}
	active_emitters.push_back(vfx::Emitter(svc, weapon.get_barrel_point(), weapon.emitter.dimensions, weapon.emitter.type, weapon.emitter.color, weapon.get_firing_direction()));
	if (weapon.secondary_emitter) {
		active_emitters.push_back(vfx::Emitter(svc, weapon.get_barrel_point(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().type, weapon.secondary_emitter.value().color, weapon.get_firing_direction()));
	}
}

void Map::spawn_enemy(int id, sf::Vector2<float> pos) {
	enemy_spawns.push_back({pos, id});
	spawn_counter.update();
	flags.state.set(LevelState::spawn_enemy);
}

void Map::manage_projectiles(automa::ServiceProvider& svc) {
	for (auto& proj : active_projectiles) {
		proj.update(svc, *player);
		if (proj.whiffed() && !proj.poofed() && !proj.made_contact()) {
			effects.push_back(entity::Effect(svc, proj.get_position(), proj.get_velocity(), proj.effect_type(), 8));
			proj.poof();
		}
	}
	for (auto& emitter : active_emitters) { emitter.update(svc, *this); }

	std::erase_if(active_projectiles, [](auto const& p) { return p.destroyed(); });

	// TODO: refactor this and move it into appropriate classes
	if (player->fire_weapon()) {
		svc.stats.player.bullets_fired.update();
		sf::Vector2<float> tweak = player->controller.facing_left() ? sf::Vector2<float>{0.f, 0.f} : sf::Vector2<float>{-3.f, 0.f};
		if (player->equipped_weapon().multishot()) {
			for (int i = 0; i < player->equipped_weapon().get_multishot(); ++i) { spawn_projectile_at(svc, player->equipped_weapon(), player->equipped_weapon().get_barrel_point()); }
		} else {
			spawn_projectile_at(svc, player->equipped_weapon(), player->equipped_weapon().get_barrel_point());
		}
		player->equipped_weapon().shoot();
		if (!player->equipped_weapon().automatic()) { player->controller.set_shot(false); }
	}
}

void Map::generate_collidable_layer(bool live) {
	auto pushable_offset = sf::Vector2<float>{1.f, 0.f};
	for (auto& cell : get_middleground().grid.cells) {
		get_middleground().grid.check_neighbors(cell.one_d_index);
		if (live) { continue; }
		if (cell.is_breakable()) { breakables.push_back(Breakable(*m_services, cell.position(), styles.breakables)); }
		if (cell.is_pushable()) { pushables.push_back(Pushable(*m_services, cell.position() + pushable_offset, styles.pushables, cell.value - 483)); }
		if (cell.is_big_spike()) { spikes.push_back(Spike(*m_services, m_services->assets.t_big_spike, cell.position(), get_middleground().grid.get_solid_neighbors(cell.one_d_index), {6.f, 4.f})); }
		if (cell.is_spike()) { spikes.push_back(Spike(*m_services, m_services->assets.tilesets.at(style_id), cell.position(), get_middleground().grid.get_solid_neighbors(cell.one_d_index), {1.f, 1.f})); }
		if (cell.is_spawner()) { spawners.push_back(Spawner(*m_services, cell.position(), 5)); }
		if (cell.is_target()) { target_points.push_back(cell.get_center()); }
		if (cell.is_checkpoint()) { checkpoints.push_back(Checkpoint(*m_services, cell.position())); }
		if (cell.is_fire()) {
			if (!fire) { fire = std::vector<Fire>{}; }
			fire.value().push_back(Fire(*m_services, cell.position(), cell.value));
		}
	}
}

void Map::generate_layer_textures(automa::ServiceProvider& svc) {
	auto& layers = svc.data.get_layers(room_id);
	if (has_obscuring_layer()) { textures.obscuring = LayerTexture(); }
	if (has_reverse_obscuring_layer()) { textures.reverse_obscuring = LayerTexture(); }
	for (auto cycle{0}; cycle < static_cast<int>(fornani::TimeOfDay::END); ++cycle) {
		for (auto& layer : layers) {
			auto changed{layer.get_i_render_order() == 0 || layer.middleground()};
			auto finished{layer.get_i_render_order() == m_middleground - 1 || layer.get_i_render_order() == layers.size() - 1};
			auto time = static_cast<fornani::TimeOfDay>(cycle);
			auto& tex{time == fornani::TimeOfDay::day ? (!layer.background() ? textures.foreground.day : textures.background.day)
													  : (time == fornani::TimeOfDay::twilight ? (!layer.background() ? textures.foreground.twilight : textures.background.twilight)
														 : !layer.background()				  ? textures.foreground.night
																							  : textures.background.night)};

			sf::Vector2u size{static_cast<unsigned int>(layer.grid.dimensions.x + barrier.x * 2.f) * static_cast<unsigned int>(svc.constants.i_cell_size),
							  static_cast<unsigned int>(layer.grid.dimensions.y + barrier.y * 2.f) * static_cast<unsigned int>(svc.constants.i_cell_size)};
			if (changed) {
				if (!tex.resize(size)) { std::cout << "Layer texture not created.\n"; }
				tex.clear(sf::Color::Transparent);
			}
			if (layer.obscuring()) {
				auto& obs_tex{time == fornani::TimeOfDay::day ? textures.obscuring.value().day : (time == fornani::TimeOfDay::twilight ? textures.obscuring.value().twilight : textures.obscuring.value().night)};
				if (!obs_tex.resize(size)) { std::cout << "Obscuring layer texture not created.\n"; }
				obs_tex.clear(sf::Color::Transparent);
			}
			if (layer.reverse_obscuring()) {
				auto& rev_obs_tex{time == fornani::TimeOfDay::day ? textures.reverse_obscuring.value().day : (time == fornani::TimeOfDay::twilight ? textures.reverse_obscuring.value().twilight : textures.reverse_obscuring.value().night)};
				if (!rev_obs_tex.resize(size)) { std::cout << "Reverse layer texture not created.\n"; }
				rev_obs_tex.clear(sf::Color::Transparent);
			}
			sf::Sprite tile{svc.assets.tilesets.at(style_id)};
			for (auto& cell : layer.grid.cells) {
				if (cell.is_occupied() && !cell.is_special()) {
					auto x_coord = static_cast<int>((cell.value % svc.constants.tileset_scaled.x) * svc.constants.i_cell_size) + (cycle * 512);
					auto y_coord = static_cast<int>(std::floor(cell.value / svc.constants.tileset_scaled.x) * svc.constants.i_cell_size);
					tile.setTextureRect(sf::IntRect({x_coord, y_coord}, {svc.constants.i_cell_size, svc.constants.i_cell_size}));
					tile.setPosition(cell.position() + scaled_barrier);
					auto normal{true};
					if (layer.obscuring()) {
						auto& obs_tex{time == fornani::TimeOfDay::day ? textures.obscuring.value().day : (time == fornani::TimeOfDay::twilight ? textures.obscuring.value().twilight : textures.obscuring.value().night)};
						obs_tex.draw(tile);
						normal = false;
					}
					if (layer.reverse_obscuring()) {
						auto& rev_obs_tex{time == fornani::TimeOfDay::day ? textures.reverse_obscuring.value().day
																		  : (time == fornani::TimeOfDay::twilight ? textures.reverse_obscuring.value().twilight : textures.reverse_obscuring.value().night)};
						rev_obs_tex.draw(tile);
						normal = false;
					}
					if (normal) { tex.draw(tile); }
					if (layer.middleground()) { draw_barrier(tex, tile, cell); }
				}
			}
			if (finished) { tex.display(); }
			if (layer.obscuring()) {
				auto& obs_tex{time == fornani::TimeOfDay::day ? textures.obscuring.value().day : (time == fornani::TimeOfDay::twilight ? textures.obscuring.value().twilight : textures.obscuring.value().night)};
				obs_tex.display();
			}
			if (layer.reverse_obscuring()) {
				auto& rev_obs_tex{time == fornani::TimeOfDay::day ? textures.reverse_obscuring.value().day : (time == fornani::TimeOfDay::twilight ? textures.reverse_obscuring.value().twilight : textures.reverse_obscuring.value().night)};
				rev_obs_tex.display();
			}
			if (layer.middleground()) {
				if (!textures.greyblock.resize(size)) { std::cout << "Layer texture not created.\n"; }
				get_middleground().grid.draw(textures.greyblock);
			}
		}
	}
}

bool Map::check_cell_collision(shape::Collider& collider, bool foreground) {
	auto& grid = foreground ? get_obscuring_layer().grid : get_middleground().grid;
	auto& layers = m_services->data.get_layers(room_id);
	auto top = get_index_at_position(collider.vicinity.vertices.at(0));
	auto bottom = get_index_at_position(collider.vicinity.vertices.at(3));
	auto right = get_index_at_position(collider.vicinity.vertices.at(1)) - top;
	for (auto i{top}; i <= bottom; i += static_cast<size_t>(dimensions.x)) {
		auto left{0};
		for (auto j{left}; j <= right; ++j) {
			auto index = i + j;
			if (index >= static_cast<unsigned long long>(dimensions.x * dimensions.y) || index < 0) { continue; }
			auto& cell = grid.get_cell(static_cast<int>(index));
			if (!cell.is_solid()) { continue; }
			cell.collision_check = true;
			if (collider.predictive_combined.SAT(cell.bounding_box)) { return true; }
		}
	}
	return false;
}

bool Map::check_cell_collision_circle(shape::CircleCollider& collider, bool collide_with_platforms) {
	auto& grid = get_middleground().grid;
	auto& layers = m_services->data.get_layers(room_id);
	auto top = get_index_at_position(collider.boundary.first);
	auto bottom = get_index_at_position(collider.boundary.second);
	auto right = static_cast<size_t>(collider.boundary_width() / m_services->constants.cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<size_t>(dimensions.x)) {
		auto left{0};
		for (auto j{left}; j <= right; ++j) {
			auto index = i + j;
			if (index >= dimensions.x * dimensions.y || index < 0) { continue; }
			auto& cell = grid.get_cell(static_cast<int>(index));
			if (!cell.is_collidable() || cell.is_ceiling_ramp()) { continue; }
			if (cell.is_platform() && !collide_with_platforms) { continue; }
			cell.collision_check = true;
			if (collider.collides_with(cell.bounding_box)) { return true; }
		}
	}
	return false;
}

void Map::handle_cell_collision(shape::CircleCollider& collider) {
	auto& grid = get_middleground().grid;
	auto top = get_index_at_position(collider.boundary.first);
	auto bottom = get_index_at_position(collider.boundary.second);
	auto right = static_cast<size_t>(collider.boundary_width() / m_services->constants.cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<size_t>(dimensions.x)) {
		auto left{0};
		for (auto j{left}; j <= right; ++j) {
			auto index = i + j;
			if (index >= dimensions.x * dimensions.y || index < 0) { continue; }
			auto& cell = grid.get_cell(static_cast<int>(index));
			if (!cell.is_collidable() || cell.is_ceiling_ramp()) { continue; }
			cell.collision_check = true;
			collider.handle_collision(cell.bounding_box);
		}
	}
}

void Map::shake_camera() { flags.state.set(LevelState::camera_shake); }

void Map::clear() {
	dimensions = {};
	portals.clear();
	platforms.clear();
	breakables.clear();
	pushables.clear();
	spikes.clear();
	destroyers.clear();
	switch_blocks.clear();
	switch_buttons.clear();
	chests.clear();
	npcs.clear();
	checkpoints.clear();
}

void Map::wrap(sf::Vector2<float>& position) const {
	if (position.x < 0.f) { position.x = real_dimensions.x; }
	if (position.y < 0.f) { position.y = real_dimensions.y; }
	if (position.x > real_dimensions.x) { position.x = 0.f; }
	if (position.y > real_dimensions.y) { position.y = 0.f; }
}

std::vector<Layer>& Map::get_layers() { return m_services->data.get_layers(room_id); }

Layer& Map::get_middleground() { return m_services->data.get_layers(room_id).at(m_middleground); }

Layer& Map::get_obscuring_layer() { return m_services->data.get_layers(room_id).at(static_cast<std::size_t>(m_services->data.get_layers(room_id).size() - 1)); }

npc::NPC& Map::get_npc(int id) {
	for (auto& npc : npcs) {
		if (npc.get_id() == id) { return npc; }
	}

	std::cerr << "Tried to get NPC that didn't exist! ID: " << id << std::endl;
	std::exit(1);
}

sf::Vector2<float> Map::get_spawn_position(int portal_source_map_id) {
	for (auto& portal : portals) {
		if (portal.get_source() == portal_source_map_id) { return (portal.position); }
	}
	return Vec(300.f, 390.f);
}

sf::Vector2<float> Map::get_nearest_target_point(sf::Vector2<float> from) {
	auto ret = sf::Vector2<float>{};
	auto dist = std::numeric_limits<float>::max();
	for (auto& target : target_points) {
		auto test = util::magnitude(from - target);
		if (test < dist) {
			ret = target;
			dist = test;
		}
	}
	return ret;
}

sf::Vector2<float> Map::last_checkpoint() {
	for (auto& checkpoint : checkpoints) {
		if (checkpoint.reached()) { return checkpoint.position(); }
	}
	return {};
}

void Map::debug() {
	background->debug();
	// for (auto& atm : atmosphere) { atm.debug(); }
}

bool Map::nearby(shape::Shape& first, shape::Shape& second) const {
	return abs(first.get_position().x + first.get_dimensions().x * 0.5f - second.get_position().x) < lookup::unit_size_f * collision_barrier && abs(first.get_position().y - second.get_position().y) < lookup::unit_size_f * collision_barrier;
}

bool Map::within_bounds(sf::Vector2<float> test) const { return test.x > 0.f && test.x < real_dimensions.x && test.y > 0.f && test.y < real_dimensions.y; }

bool Map::overlaps_middleground(shape::Shape& test) {
	for (auto& cell : get_middleground().grid.cells) {
		if (test.overlaps(cell.bounding_box) && cell.is_solid()) { return true; }
	}
	return false;
}

std::size_t Map::get_index_at_position(sf::Vector2<float> position) { return get_middleground().grid.get_index_at_position(position); }

int Map::get_tile_value_at_position(sf::Vector2<float> position) { return get_middleground().grid.get_cell(get_index_at_position(position)).value; }

Tile& Map::get_cell_at_position(sf::Vector2<float> position) { return get_middleground().grid.cells.at(get_index_at_position(position)); }

void world::Map::draw_barrier(sf::RenderTexture& tex, sf::Sprite& tile, Tile& cell) {
	sf::Vector2f offset = 2.f * scaled_barrier;
	if (cell.index.x == 0) {
		tile.setPosition(cell.position());
		tex.draw(tile);
	}
	if (cell.index.y == 0) {
		tile.setPosition(cell.position());
		tex.draw(tile);
	}
	if (cell.index.x == dimensions.x - 1) {
		tile.setPosition(cell.position() + offset);
		tex.draw(tile);
	}
	if (cell.index.y == dimensions.y - 1) {
		tile.setPosition(cell.position() + offset);
		tex.draw(tile);
	}
	if (cell.index.x == 0 && cell.index.y == dimensions.y - 1) {
		tile.setPosition(cell.position() + sf::Vector2f{0.f, scaled_barrier.y});
		tex.draw(tile);
		tile.setPosition(cell.position() + sf::Vector2f{0.f, 2.f * scaled_barrier.y});
		tex.draw(tile);
		tile.setPosition(cell.position() + sf::Vector2f{scaled_barrier.x, 2.f * scaled_barrier.y});
		tex.draw(tile);
	}
	if (cell.index.y == 0 && cell.index.x == dimensions.x - 1) {
		tile.setPosition(cell.position() + sf::Vector2f{scaled_barrier.x, 0.f});
		tex.draw(tile);
		tile.setPosition(cell.position() + sf::Vector2f{2.f * scaled_barrier.x, 0.f});
		tex.draw(tile);
		tile.setPosition(cell.position() + sf::Vector2f{2.f * scaled_barrier.x, scaled_barrier.y});
		tex.draw(tile);
	}
}

} // namespace fornani::world
