
#include <imgui.h>
#include <ccmath/ext/clamp.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/gui/Portrait.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/EnumLookups.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>
#include <ranges>

static bool b_transition_in{};

namespace fornani::world {

Map::Map(automa::ServiceProvider& svc, player::Player& player)
	: player(&player), enemy_catalog(svc), transition(svc.window->f_screen_dimensions(), 96), m_services(&svc), cooldowns{.fade_obscured{util::Cooldown(128)}, .loading{util::Cooldown(24)}} {}

Map::~Map() {}

void Map::load(automa::ServiceProvider& svc, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, int room_number) {

	unserialize(svc, room_number);

	auto it = std::find_if(svc.data.map_jsons.begin(), svc.data.map_jsons.end(), [room_number](auto const& r) { return r.id == room_number; });
	if (it == svc.data.map_jsons.end()) { return; }

	auto& metadata = it->metadata;
	auto const& meta = metadata["meta"];
	auto& entities = metadata["entities"];

	// process properties
	svc.music_player.load(svc.finder, m_attributes.music);
	svc.music_player.play_looped();
	svc.ambience_player.load(svc.finder, m_attributes.ambience);
	svc.ambience_player.play();
	for (auto const& atmo : m_attributes.atmosphere) { atmosphere.push_back(vfx::Atmosphere(svc, real_dimensions, atmo)); }
	background = std::make_unique<graphics::Background>(svc, meta["background"].as_string());

	svc.current_room = room_number;
	if (meta["cutscene_on_entry"]["flag"].as_bool()) {
		auto ctype = meta["cutscene_on_entry"]["type"].as<int>();
		auto cid = meta["cutscene_on_entry"]["id"].as<int>();
		auto csource = meta["cutscene_on_entry"]["source"].as<int>();
		auto cutscene = util::QuestKey{ctype, cid, csource};
		svc.quest.process(svc, cutscene);
		cutscene_catalog.push_cutscene(svc, *this, cid);
	}
	for (auto& pl : entities["lights"].as_array()) {
		point_lights.push_back(PointLight(svc.data.light[pl["label"].as_string()], sf::Vector2f{pl["position"][0].as<float>() + 0.5f, pl["position"][1].as<float>() + 0.5f} * constants::f_cell_size));
	}

	if (meta["camera_effects"]) {
		m_camera_effects.shake_properties.frequency = meta["camera_effects"]["shake"]["frequency"].as<int>();
		m_camera_effects.shake_properties.energy = meta["camera_effects"]["shake"]["energy"].as<float>();
		m_camera_effects.shake_properties.start_time = meta["camera_effects"]["shake"]["start_time"].as<float>();
		m_camera_effects.shake_properties.dampen_factor = meta["camera_effects"]["shake"]["dampen_factor"].as<int>();
		m_camera_effects.cooldown = util::Cooldown{meta["camera_effects"]["shake"]["frequency_in_seconds"].as<int>()};
		m_camera_effects.shake_properties.shaking = m_camera_effects.shake_properties.frequency > 0;
		m_camera_effects.cooldown.start();
	}

	sound.echo_count = meta["sound"]["echo_count"].as<int>();
	sound.echo_rate = meta["sound"]["echo_rate"].as<int>();

	if (meta["weather"]["rain"]) { rain = vfx::Rain(meta["weather"]["rain"]["intensity"].as<int>(), meta["weather"]["rain"]["fall_speed"].as<float>(), meta["weather"]["rain"]["slant"].as<float>()); }
	if (meta["weather"]["snow"]) { rain = vfx::Rain(meta["weather"]["snow"]["intensity"].as<int>(), meta["weather"]["snow"]["fall_speed"].as<float>(), meta["weather"]["snow"]["slant"].as<float>(), true); }
	if (meta["weather"]["leaves"]) { rain = vfx::Rain(meta["weather"]["leaves"]["intensity"].as<int>(), meta["weather"]["leaves"]["fall_speed"].as<float>(), meta["weather"]["leaves"]["slant"].as<float>(), true, true); }

	for (auto& entry : entities["chests"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		chests.push_back(std::make_unique<entity::Chest>(svc, *this, entry["id"].as<int>(), static_cast<entity::ChestType>(entry["type"].as<int>()), entry["modifier"].as<int>()));
		chests.back()->set_position_from_scaled(pos);
	}

	for (auto& entry : entities["animators"].as_array()) {
		sf::Vector2i scaled_pos{};
		scaled_pos.x = entry["position"][0].as<int>();
		scaled_pos.y = entry["position"][1].as<int>();
		animators.push_back(entity::Animator(svc, entry["label"].as_string(), entry["id"].as<int>(), scaled_pos, entry["foreground"].as_bool()));
	}
	for (auto& entry : entities["beds"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>() * constants::f_cell_size;
		pos.y = entry["position"][1].as<float>() * constants::f_cell_size;
		auto flipped = static_cast<bool>(entry["flipped"].as_bool());
		beds.push_back(entity::Bed(svc, pos, m_biome.get_id(), flipped));
	}
	for (auto& entry : entities["scenery"]["basic"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>() * constants::f_cell_size;
		pos.y = entry["position"][1].as<float>() * constants::f_cell_size;
		auto var = entry["variant"].as<int>();
		auto lyr = entry["layer"].as<int>();
		auto parallax = entry["parallax"].as<float>();
		scenery_layers.at(lyr).push_back(std::make_unique<vfx::Scenery>(svc, pos, m_biome.get_id(), lyr, var, parallax));
	}
	for (auto [i, entry] : std::views::enumerate(entities["inspectables"].as_array())) {
		auto push = true;
		auto fail_tag = std::string{};
		if (entry["contingencies"].is_array()) {
			for (auto const& contingency : entry["contingencies"].as_array()) {
				auto cont = QuestContingency{contingency};
				if (!svc.quest_table.are_contingencies_met({cont})) {
					push = false;
					fail_tag = contingency["tag"].as_string();
				}
			}
		}
		if (push) { inspectables.push_back(entity::Inspectable(svc, entry, room_id, i)); }
		if (svc.data.inspectable_is_destroyed(inspectables.back().get_id())) { inspectables.back().destroy(); }
	}

	for (auto& entry : entities["destructibles"].as_array()) { destructibles.push_back(std::make_unique<Destructible>(svc, *this, entry, m_biome.get_id())); }

	for (auto& entry : entities["enemies"].as_array()) {
		int id{};
		sf::Vector2f pos{};
		sf::Vector2<int> start{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		start.x = entry["start_direction"][0].as<int>();
		start.y = entry["start_direction"][1].as<int>();
		auto variant = entry["variant"].as<int>();
		enemy_catalog.push_enemy(svc, *this, console, entry["id"].as<int>(), false, variant, start);
		enemy_catalog.enemies.back()->set_position_from_scaled({pos * constants::f_cell_size});
		enemy_catalog.enemies.back()->get_collider().physics.zero();
		enemy_catalog.enemies.back()->set_external_id({room_id, {static_cast<int>(pos.x), static_cast<int>(pos.y)}});
		if (svc.data.enemy_is_fallen(room_id, enemy_catalog.enemies.back()->get_external_id())) { enemy_catalog.enemies.pop_back(); }
	}

	for (auto& entry : entities["platforms"].as_array()) {
		sf::Vector2f dim{};
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		dim.x = entry["dimensions"][0].as<float>();
		dim.y = entry["dimensions"][1].as<float>();
		pos *= constants::f_cell_size;
		dim *= constants::f_cell_size;
		auto start = entry["start"].as<float>();
		start = ccm::ext::clamp(start, 0.f, 1.f);
		auto type = entry["type"].as_string();
		platforms.push_back(std::make_unique<Platform>(svc, *this, pos, dim, entry["extent"].as<float>(), type, start, entry["style"].as<int>()));
	}
	for (auto& entry : entities["switch_blocks"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		pos *= constants::f_cell_size;
		auto type = entry["type"].as<int>();
		auto button_id = entry["id"].as<int>();
		switch_blocks.push_back(std::make_unique<SwitchBlock>(svc, *this, pos, button_id, type));
	}
	for (auto& entry : entities["switch_buttons"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		pos *= constants::f_cell_size;
		auto type = entry["type"].as<int>();
		auto button_id = entry["id"].as<int>();
		switch_buttons.push_back(std::make_unique<SwitchButton>(svc, pos, button_id, type, *this));
	}
	for (auto& entry : entities["timer_blocks"].as_array()) {
		sf::Vector2<std::uint32_t> pos{};
		pos.x = entry["position"][0].as<std::uint32_t>();
		pos.y = entry["position"][1].as<std::uint32_t>();
		auto type = static_cast<TimerBlockType>(entry["type"].as<int>());
		timer_blocks.push_back(TimerBlock(svc, pos, type, entry["id"].as<int>()));
	}

	generate_layer_textures(svc);
	cooldowns.fade_obscured.start();
	player->map_reset();
	transition.set(graphics::TransitionState::black);
	cooldowns.loading.start();

	b_transition_in = true;

	player->register_with_map(*this);
	NANI_LOG_INFO(m_logger, "Player registered with map.");
}

void Map::unserialize(automa::ServiceProvider& svc, int room_number, bool live) {

	NANI_LOG_DEBUG(m_logger, "Unserializing map...");

	auto it = std::find_if(svc.data.map_jsons.begin(), svc.data.map_jsons.end(), [room_number](auto const& r) { return r.id == room_number; });
	if (it == svc.data.map_jsons.end()) {
		NANI_LOG_WARN(m_logger, "Map json not found!");
		return;
	}

	// fetch template data
	auto const& in_template = svc.data.map_templates;
	auto bit = std::find_if(svc.data.map_templates.begin(), svc.data.map_templates.end(), [it](auto const& r) { return r.biome_label == it->biome_label; });
	if (bit == svc.data.map_templates.end()) {
		NANI_LOG_WARN(m_logger, "Tried to fetch template data for a nonexistent biome!");
	} else {
		m_attributes = MapAttributes{bit->metadata};
	}

	auto& metadata = it->metadata;
	auto const& meta = metadata["meta"];
	auto& entities = metadata["entities"];

	use_template = meta["use_template"].as_bool();
	if (!use_template) { m_attributes = MapAttributes{meta}; } // install metadata override
	if (meta["minimap"].as_bool()) { m_attributes.properties.set(MapProperties::minimap); }

	center_box.setSize(svc.window->f_screen_dimensions() * 0.5f);
	flags.state.reset(LevelState::game_over);
	if (!player->is_dead()) { svc.state_controller.actions.reset(automa::Actions::death_mode); }
	spawn_counter.start();

	svc.state_flags.reset(automa::StateFlags::hide_hud);

	svc.world_timer.set_tag("nani");
	svc.world_timer.set_course(room_number);

	m_biome = svc.data.construct_biome(it->biome_label);
	m_metadata.room = it->room_label;

	// check for enemy respawns
	svc.data.respawn_enemies(room_id, player->visit_history.distance_traveled_from(room_id));

	room_id = meta["room_id"].as<int>();
	metagrid_coordinates.x = meta["metagrid"][0].as<int>();
	metagrid_coordinates.y = meta["metagrid"][1].as<int>();
	dimensions.x = meta["dimensions"][0].as<int>();
	dimensions.y = meta["dimensions"][1].as<int>();
	m_player_start.x = meta["player_start"][0].as<float>();
	m_player_start.y = meta["player_start"][1].as<float>();
	real_dimensions = {static_cast<float>(dimensions.x) * constants::f_cell_size, static_cast<float>(dimensions.y) * constants::f_cell_size};

	if (m_attributes.properties.test(MapProperties::lighting)) {
		m_palette = Palette{get_biome_string(), svc.finder};
		darken_factor = meta["shader"]["darken_factor"].as<float>();
		NANI_LOG_DEBUG(m_logger, "Map darken factor: {}", darken_factor);
		if (m_palette->get_size() == 0) {
			m_attributes.properties.reset(MapProperties::lighting);
			m_palette = {};
			NANI_LOG_WARN(m_logger, "Map Properties > Lighting is TRUE but there exists no palette for the style!");
		}
	}

	m_middleground = metadata["tile"]["middleground"].as<int>();
	metadata["tile"]["flags"]["obscuring"].as_bool() ? m_layer_properties.set(LayerProperties::has_obscuring_layer) : m_layer_properties.reset(LayerProperties::has_obscuring_layer);
	metadata["tile"]["flags"]["reverse_obscuring"].as_bool() ? m_layer_properties.set(LayerProperties::has_reverse_obscuring_layer) : m_layer_properties.reset(LayerProperties::has_reverse_obscuring_layer);

	m_chunks.resize(static_cast<std::size_t>((dimensions.x / constants::u32_chunk_size) * (dimensions.y / constants::u32_chunk_size)));

	m_attributes.border_color = m_biome.get_id() == 2 ? colors::pioneer_black : colors::ui_black;
	if (entities.is_object()) { m_entities = EntitySet(svc, *this, svc.finder, entities, m_metadata.room); }

	generate_collidable_layer(live);
}

void Map::update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console) {
	if (!player->has_collider()) { return; }
	auto& layers = svc.data.get_layers(room_id);
	flags.state.reset(LevelState::camera_shake);

	if (b_transition_in && transition.has_waited(64)) {
		transition.end();
		b_transition_in = false;
	}

	// camera effects
	if (svc.ticker.every_second() && m_camera_effects.shake_properties.shaking) {
		m_camera_effects.cooldown.update();
		if (m_camera_effects.cooldown.is_complete()) {
			svc.camera_controller.shake(m_camera_effects.shake_properties);
			auto shake_time = m_camera_effects.cooldown.get_native_time();
			auto diff = random::random_range(-shake_time / 2, shake_time / 2);
			m_camera_effects.cooldown.start(shake_time + diff);
		}
	}

	if (flags.state.test(LevelState::spawn_enemy)) {
		for (auto& spawn : enemy_spawns) {
			enemy_catalog.push_enemy(*m_services, *this, console, spawn.id, true, spawn.variant);
			enemy_catalog.enemies.back()->intangible_start(64);
			enemy_catalog.enemies.back()->set_position(spawn.pos);
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			spawn_effect(svc, "small_flash", enemy_catalog.enemies.back()->get_collider().get_center());
		}
		enemy_spawns.clear();
		flags.state.reset(LevelState::spawn_enemy);
	}

	if (off_the_bottom(player->get_collider().physics.position) && cooldowns.loading.is_complete()) {
		player->hurt(64.f);
		player->freeze_position();
	}

	// hidden areas
	flags.map_state.test(MapState::unobscure) ? cooldowns.fade_obscured.update() : cooldowns.fade_obscured.reverse();
	if (check_cell_collision(player->get_collider(), true)) {
		if (!flags.map_state.test(MapState::unobscure)) { cooldowns.fade_obscured.start(); }
		if (cooldowns.loading.running()) { cooldowns.fade_obscured.cancel(); }
		flags.map_state.set(MapState::unobscure);
	} else {
		if (flags.map_state.test(MapState::unobscure)) { cooldowns.fade_obscured.cancel(); }
		flags.map_state.reset(MapState::unobscure);
	}

	std::erase_if(active_emitters, [](auto const& p) { return p->done(); });
	std::erase_if(effects, [](auto& e) { return e.done(); });
	std::erase_if(inspectables, [](auto const& i) { return i.destroyed(); });
	std::erase_if(lasers, [](auto const& l) { return l.is_complete(); });
	std::erase_if(incinerite_blocks, [](auto const& i) { return i->is_destroyed(); });
	std::erase_if(breakables, [](auto const& b) { return b->is_destroyed(); });
	enemy_catalog.update();

	manage_projectiles(svc);
	svc.map_debug.active_projectiles = active_projectiles.size();
	for (auto& proj : active_projectiles) {
		if (proj.destruction_initiated()) { continue; }
		proj.register_chunk(get_chunk_id_from_position(proj.get_position()));
		for (auto& platform : platforms) { platform->on_hit(svc, *this, proj); }
		for (auto& breakable : breakables) { breakable->on_hit(svc, *this, proj); }
		for (auto& pushable : pushables) { pushable->on_hit(svc, *this, proj); }
		for (auto& destructible : destructibles) { destructible->on_hit(svc, *this, proj); }
		for (auto& block : switch_blocks) { block->on_hit(svc, *this, proj); }
		for (auto& enemy : enemy_catalog.enemies) { enemy->on_hit(svc, *this, proj); }
		for (auto& incinerite : incinerite_blocks) { incinerite->on_hit(svc, *this, proj); }
		for (auto vine : get_entities<Vine>()) { vine->on_hit(svc, *this, proj, *player); }
		proj.handle_collision(svc, *this);
		proj.on_player_hit(*player);
	}

	for (auto& enemy : enemy_catalog.enemies) { enemy->update(svc, *this, *player); }
	for (auto& emitter : active_emitters) { emitter->update(svc, *this); }
	for (auto& platform : platforms) { platform->update(svc, *this, *player); }

	num_collision_checks = 0;
	for (auto& colliderPtr : m_colliders) {
		if (!colliderPtr) { continue; }
		auto& collider = *colliderPtr;
		collider.register_chunks(*this);
		collider.update(svc);
		collider.detect_map_collision(*this);
		for (auto chunk : collider.get_chunks()) {
			for (auto& other_ptr : m_chunks[chunk]) {
				if (!other_ptr) { continue; }
				if (other_ptr == &collider) { continue; }
				if (collider.is_intangible()) { continue; }
				collider.handle_collision(*other_ptr);
				++num_collision_checks;
			}
		}
	}

	for (auto& enemy : enemy_catalog.enemies) { enemy->post_update(svc, *this, *player); }
	for (auto& pushable : pushables) { pushable->update(svc, *this, *player); }
	for (auto& platform : platforms) { platform->post_update(svc, *this, *player); }

	player->on_crush(*this);
	for (auto& enemy : enemy_catalog.enemies) { enemy->on_crush(*this); }

	if (m_entities) {
		for (auto& entity : m_entities.value().variables.entities) { entity->update(svc, *this, console, *player); }
	}
	if (fire) {
		for (auto& f : fire.value()) { f.update(svc, *player, *this, console); }
	}
	for (auto& laser : lasers) { laser.update(svc, *player, *this); }
	for (auto& loot : active_loot) { loot.update(svc, *this, *player); }
	for (auto& chest : chests) { chest->update(svc, *this, console, *player); }
	/*for (auto& npc : npcs) {
		npc->update(svc, *this, console, *player);
	}*/
	for (auto& cutscene : cutscene_catalog.cutscenes) { cutscene->update(svc, console, *this, *player); }
	// for (auto& portal : portals) { portal.handle_activation(svc, *player, console, room_id, transition); }
	for (auto& inspectable : inspectables) { inspectable.update(svc, *this, console, *player); }
	for (auto& animator : animators) { animator.update(); }
	for (auto& effect : effects) { effect.update(); }
	for (auto& atm : atmosphere) { atm.update(svc, *this, *player); }
	for (auto& spawner : spawners) { spawner.update(svc, *this); }
	for (auto& switch_block : switch_blocks) { switch_block->update(svc, *this, *player); }
	for (auto& switch_button : switch_buttons) { switch_button->update(svc, *this, *player); }
	for (auto& destructible : destructibles) { destructible->update(svc, *this, *player); }
	for (auto& checkpoint : checkpoints) { checkpoint.update(svc, *this, *player); }
	for (auto& bed : beds) { bed.update(svc, *this, console, *player, transition); }
	for (auto& breakable : breakables) { breakable->update(svc, *this, *player); }
	for (auto& incinerite : incinerite_blocks) { incinerite->update(svc, *this, *player); }

	for (auto& pushable : pushables) { pushable->post_update(svc, *this, *player); }
	for (auto& spike : spikes) { spike.update(svc, *player, *this); }
	// for (auto& vine : vines) { vine->update(svc, *this, *player); }
	for (auto& timer_block : timer_blocks) { timer_block.update(svc, *this, *player); }
	for (auto& pl : point_lights) { pl.update(); }
	if (cooldowns.loading.is_complete()) { transition.update(*player); }
	if (player->get_collider().collision_depths) { player->get_collider().collision_depths.value().update(); }
	// if (save_point) { save_point->update(svc, *player, console); }
	if (rain) { rain.value().update(svc, *this); }

	player->get_collider().reset_ground_flags();

	// check if player died
	if (!flags.state.test(LevelState::game_over) && player->death_animation_over() && svc.death_mode() && cooldowns.loading.is_complete()) {
		svc.app_flags.reset(automa::AppFlags::in_game);
		console = std::make_unique<gui::Console>(svc, svc.text.basic, "death", gui::OutputType::gradual);
		flags.state.set(LevelState::game_over);
		svc.music_player.load(svc.finder, "mortem");
		svc.music_player.play_looped();
		svc.soundboard.turn_off();
		svc.stats.player.death_count.update();
	}

	// demo only
	if (svc.state_controller.actions.consume(automa::Actions::end_demo)) { end_demo.start(); }
	end_demo.update();
	if (end_demo.get() == 1) { console = std::make_unique<gui::Console>(svc, svc.text.basic, "end_demo", gui::OutputType::gradual); }
	if (svc.state_controller.actions.test(automa::Actions::print_stats) && !console) { svc.state_controller.actions.set(automa::Actions::trigger); }
	// demo only

	if (svc.state_controller.actions.test(automa::Actions::retry)) { flags.state.set(LevelState::game_over); }
	if (!console && flags.state.test(LevelState::game_over)) {
		if (!flags.state.test(LevelState::transitioning)) { transition.start(); }
		flags.state.set(LevelState::transitioning);
		if (transition.is(graphics::TransitionState::black)) {
			player->start_over();
			svc.state_controller.actions.set(automa::Actions::player_death);
			svc.state_controller.actions.set(automa::Actions::trigger);
		}
	}
	cooldowns.loading.update();
}

void Map::render(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam) {
	// check for a switch to greyblock mode
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_trigger)) {
		generate_layer_textures(svc);
		svc.debug_flags.reset(automa::DebugFlags::greyblock_trigger);
	}

	if (m_entities) {
		// TODO: uncomment below once all entities have been refactored!
		// for (auto& entity : m_entities.value().variables.entities) { entity->render(win, cam, 1.0); }
		for (auto p : get_entities<Portal>()) { p->render(win, cam, 1.0); }
		for (auto s : get_entities<SavePoint>()) { s->render(win, cam, 1.0); }
		for (auto n : get_entities<NPC>()) {
			if (!n->is_background()) { n->render(win, cam); }
		}
		if (svc.greyblock_mode()) {
			for (auto c : get_entities<CutsceneTrigger>()) { c->render(win, cam, c->get_f_grid_dimensions().x); }
		}
		for (auto v : get_entities<Vine>()) {
			if (!v->is_foreground()) { v->render(win, cam, 1.0); }
		}
		// for (auto n : get_entities<NPC>()) { n->render(win, cam, 1.0); }
	}

	// for (auto& portal : portals) { portal.render(svc, win, cam); }
	if (fire) {
		for (auto& f : fire.value()) { f.render(svc, win, cam); }
	}
	for (auto& bed : beds) { bed.render(svc, win, cam); }
	for (auto& chest : chests) { chest->render(win, cam); }
	/*for (auto& npc : npcs) {
		if (!npc->background()) { npc->render(svc, win, cam); }
	}*/
	player->render(svc, win, cam);
	for (auto& enemy : enemy_catalog.enemies) {
		if (!enemy->is_foreground()) { enemy->render(svc, win, cam); }
	}
	for (auto& proj : active_projectiles) { proj.render(svc, *player, win, cam); }
	for (auto& loot : active_loot) { loot.render(svc, win, cam); }
	for (auto& emitter : active_emitters) { emitter->render(svc, win, cam); }
	for (auto& platform : platforms) { platform->render(svc, win, cam); }
	for (auto& breakable : breakables) { breakable->render(svc, win, cam); }
	for (auto& incinerite : incinerite_blocks) { incinerite->render(svc, win, cam); }
	for (auto& pushable : pushables) { pushable->render(svc, win, cam); }
	for (auto& checkpoint : checkpoints) { checkpoint.render(svc, win, cam); }
	for (auto& spike : spikes) { spike.render(svc, win, shader, m_palette, cam); }
	for (auto& switch_block : switch_blocks) { switch_block->render(svc, win, cam); }
	for (auto& switch_button : switch_buttons) { switch_button->render(svc, win, cam); }
	for (auto& atm : atmosphere) { atm.render(svc, win, cam); }

	if (!svc.greyblock_mode()) {
		for (auto& layer : get_layers()) {
			if (m_attributes.properties.test(MapProperties::lighting) && m_palette && shader && !layer->ignore_lighting()) {
				shader->finalize();
				layer->render(svc, win, shader.value(), m_palette.value(), m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, false, m_attributes.properties.test(MapProperties::day_night_shift));
			} else {
				layer->render(svc, win, m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, false, m_attributes.properties.test(MapProperties::day_night_shift));
			}
		}
	}

	for (auto& destructible : destructibles) { destructible->render(svc, win, cam); }

	if (m_entities) {
		for (auto v : get_entities<Vine>()) {
			if (v->is_foreground()) { v->render(win, cam, 1.0); }
		}
		for (auto t : get_entities<Turret>()) { t->render(win, cam, 1.0); }
	}

	// foreground enemies
	for (auto& enemy : enemy_catalog.enemies) {
		if (enemy->is_foreground()) { enemy->render(svc, win, cam); }
		enemy->render_indicators(svc, win, cam);
		enemy->gui_render(svc, win, cam);
	}

	for (auto& laser : lasers) { laser.render(svc, win, cam); }
	for (auto& effect : effects) { effect.render(win, cam); }

	player->render_indicators(svc, win, cam);

	for (auto& animator : animators) {
		if (animator.is_foreground()) { animator.render(win, cam); }
	}

	for (auto& inspectable : inspectables) { inspectable.render(svc, win, cam); }

	if (rain) { rain->render(svc, win, cam); }

	if (m_attributes.properties.test(MapProperties::timer)) { svc.world_timer.render(win, sf::Vector2f{32.f, 32.f}); }

	if (svc.greyblock_mode()) {
		center_box.setPosition({});
		center_box.setFillColor(sf::Color(80, 80, 80, 60));
		win.draw(center_box);
		center_box.setPosition(svc.window->f_screen_dimensions() * 0.5f);
		win.draw(center_box);
		center_box.setFillColor(sf::Color(100, 100, 100, 60));
		center_box.setPosition({svc.window->f_screen_dimensions().x * 0.5f, 0.f});
		win.draw(center_box);
		center_box.setPosition({0.f, svc.window->f_screen_dimensions().y * 0.5f});
		win.draw(center_box);
		get_middleground()->grid.render(win, cam);
	}
}

void Map::render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam) {
	if (!svc.greyblock_mode()) {
		background->render(svc, win, cam);
		for (auto& layer : scenery_layers) {
			for (auto& piece : layer) { piece->render(svc, win, cam); }
		}
		if (!svc.greyblock_mode()) {
			for (auto [i, layer] : std::views::enumerate(get_layers())) {
				if (i == 1) {
					if (m_entities) {
						for (auto n : get_entities<NPC>()) {
							if (n->is_background()) { n->render(win, cam); }
						}
					}
				}
				if (m_attributes.properties.test(MapProperties::lighting) && m_palette && shader && !layer->ignore_lighting()) {
					shader->finalize();
					layer->render(svc, win, shader.value(), m_palette.value(), m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, true);
				} else {
					layer->render(svc, win, m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, true, m_attributes.properties.test(MapProperties::day_night_shift));
				}
			}
		}
		for (auto& switch_block : switch_blocks) { switch_block->render(svc, win, cam, true); }
	} else {
		sf::RectangleShape box{};
		box.setFillColor(colors::black);
		box.setSize(svc.window->f_screen_dimensions());
		win.draw(box);
	}

	/*for (auto& vine : vines) {
		if (!vine->foreground()) { vine->render(svc, win, cam); }
	}*/
	for (auto& animator : animators) {
		if (!animator.is_foreground()) { animator.render(win, cam); }
	}
	for (auto& timer_block : timer_blocks) { timer_block.render(svc, win, cam); }
}

bool Map::handle_entry(player::Player& player, util::Cooldown& enter_room) {
	auto ret = false;
	if (!m_entities) { return false; }
	for (auto const& entity : m_entities.value().variables.entities) {
		if (auto* portal = dynamic_cast<Portal*>(entity.get())) {
			if (portal->get_destination() == m_services->state_controller.source_id) {
				ret = true;
				sf::Vector2f spawn_position{portal->get_world_position().x + (portal->get_world_dimensions().x * 0.5f), portal->get_world_position().y + portal->get_world_dimensions().y - player.height()};
				player.set_position(spawn_position, true);
				player.force_camera_center();
				if (portal->is_activate_on_contact() && portal->is_left_or_right()) {
					enter_room.start();
				} else {
					if (!portal->is_already_open()) { portal->close(); }
					player.set_idle();
				}
				if (portal->is_bottom()) { player.get_collider().physics.acceleration.y = -player.physics_stats.jump_velocity; }
			}
		}
	}
	return ret;
}

auto Map::get_chunk_id_from_position(sf::Vector2f pos) const -> std::uint8_t {
	auto clamped = sf::Vector2f{std::clamp(pos.x, 0.f, real_dimensions.x), std::clamp(pos.y, 0.f, real_dimensions.y)};
	auto clookup = (clamped / constants::f_cell_size) / constants::f_chunk_size;
	auto ulookup = sf::Vector2u{clookup};
	auto ret = std::clamp(ulookup.y * get_chunk_dimensions().x + ulookup.x, 0u, static_cast<unsigned int>(m_chunks.size() - 1));
	return static_cast<std::uint8_t>(ret);
}

void Map::spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2f pos, sf::Vector2f target, float speed_multiplier) {
	active_projectiles.push_back(weapon.projectile);
	active_projectiles.back().set_position(pos);
	active_projectiles.back().seed(svc, target, speed_multiplier);
	active_projectiles.back().update(svc, *player);
	active_projectiles.back().register_chunk(get_chunk_id_from_position(pos));

	if (weapon.secondary_emitter) { spawn_emitter(svc, weapon.secondary_emitter.value().type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().color); }
	spawn_emitter(svc, weapon.emitter.type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.emitter.dimensions, weapon.emitter.color);
	if (weapon.secondary_emitter) { spawn_emitter(svc, weapon.secondary_emitter.value().type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().color); }
}

void Map::spawn_effect(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2f pos, sf::Vector2f vel, int channel) { effects.push_back(entity::Effect(svc, tag.data(), pos, vel, channel)); }

void Map::spawn_emitter(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2f pos, Direction dir, sf::Vector2f dim, sf::Color color) {
	active_emitters.push_back(std::make_unique<vfx::Emitter>(svc, *this, pos, dim, tag, color, dir));
}

void Map::spawn_enemy(int id, sf::Vector2f pos, int variant) {
	auto break_out = 0;
	while (player->distant_vicinity.contains_point(pos) && break_out < 32) {
		auto distance = player->get_collider().get_center() - pos;
		pos -= distance;
		++break_out;
	}
	enemy_spawns.push_back({pos, id, variant});
	spawn_counter.update();
	flags.state.set(LevelState::spawn_enemy);
}

void Map::reveal_npc(std::string_view label) {
	if (!m_entities) {
		NANI_LOG_DEBUG(m_logger, "No NPCs to reveal...");
		NANI_LOG_DEBUG(m_logger, "Label was {}", label);
		return;
	}
	for (auto n : get_entities<NPC>()) {
		NANI_LOG_DEBUG(m_logger, "Trying {}", n->get_tag());
		if (n->get_tag() == label) {
			n->unhide();
			NANI_LOG_DEBUG(m_logger, "Unhid NPC with label '{}'.", label);
		}
	}
}

void Map::manage_projectiles(automa::ServiceProvider& svc) {
	for (auto& proj : active_projectiles) {
		proj.update(svc, *player);
		if (proj.whiffed() && !proj.poofed() && !proj.made_contact()) {
			effects.push_back(entity::Effect(svc, "bullet_whiff", proj.get_position(), proj.get_velocity() * 0.05f, proj.effect_type()));
			proj.poof();
		}
	}

	std::erase_if(active_projectiles, [](auto const& p) { return p.destroyed(); });

	// TODO: refactor this and move it into appropriate classes
	if (player->fire_weapon()) {
		svc.stats.player.bullets_fired.update();
		sf::Vector2f tweak = player->controller.facing_left() ? sf::Vector2f{0.f, 0.f} : sf::Vector2f{-3.f, 0.f};
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
	auto pushable_offset = sf::Vector2f{1.f, 0.f};
	for (auto& cell : get_middleground()->grid.cells) {
		auto chunk_id = cell.get_chunk_id();
		get_middleground()->grid.check_neighbors(cell.one_d_index);
		if (live) { continue; }
		if (cell.is_breakable()) { breakables.push_back(std::make_unique<Breakable>(*m_services, *this, cell.position())); }
		if (cell.is_pushable()) { pushables.push_back(std::make_unique<Pushable>(*m_services, *this, cell.position() + pushable_offset, get_style_id(), cell.value - 483)); }
		if (cell.is_big_spike()) {
			spikes.push_back(Spike(*m_services, m_services->assets.get_texture("big_spike"), cell.position(), get_middleground()->grid.get_solid_neighbors(cell.one_d_index), {6.f, 4.f}, m_biome.get_id(),
								   m_attributes.properties.test(MapProperties::environmental_randomness)));
		}
		if (cell.is_spike()) {
			spikes.push_back(Spike(*m_services, m_services->assets.get_tileset(std::string{get_biome_string()}), cell.position(), get_middleground()->grid.get_solid_neighbors(cell.one_d_index), {1.f, 1.f}, m_biome.get_id(),
								   m_attributes.properties.test(MapProperties::environmental_randomness)));
		}
		if (cell.is_spawner()) { spawners.push_back(Spawner(*m_services, cell.position(), 5)); }
		if (cell.is_target()) { target_points.push_back(cell.get_global_center()); }
		if (cell.is_home()) { home_points.push_back(cell.get_global_center()); }
		if (cell.is_incinerite()) { incinerite_blocks.push_back(std::make_unique<Incinerite>(*m_services, *this, cell.position(), chunk_id)); }
		if (cell.is_checkpoint()) { checkpoints.push_back(Checkpoint(*m_services, cell.position())); }
		if (cell.is_fire()) {
			if (!fire) { fire = std::vector<Fire>{}; }
			fire.value().push_back(Fire(*m_services, cell.position(), cell.value));
		}
	}
}

void Map::generate_layer_textures(automa::ServiceProvider& svc) const {
	for (auto& layer : svc.data.get_layers(room_id)) { layer->generate_textures(svc.assets.get_tileset(std::string{get_biome_string()})); }
}

void Map::register_collider(std::unique_ptr<shape::ICollider> collider) {
	auto chunk_ids = collider->compute_chunks(*this);
	m_colliders.push_back(std::move(collider));
	auto collider_ptr = m_colliders.back().get();
	for (auto id : chunk_ids) { m_chunks[id].push_back(collider_ptr); }
}

void Map::unregister_collider(shape::ICollider* collider) {
	std::erase_if(m_colliders, [&](auto& ptr) { return ptr.get() == collider; });
	for (auto& chunk : m_chunks) { chunk.erase(std::remove(chunk.begin(), chunk.end(), collider), chunk.end()); }
}

void Map::refresh_collider_chunks(Register<int> const& old_chunks, Register<int> const& new_chunks, shape::ICollider* ptr) {
	for (auto chunk : old_chunks) {
		if (!new_chunks.contains(chunk)) {
			auto& bucket = m_chunks[chunk];
			bucket.erase(std::remove(bucket.begin(), bucket.end(), ptr), bucket.end());
		}
	}

	for (auto chunk : new_chunks) {
		if (!old_chunks.contains(chunk)) { m_chunks[chunk].push_back(ptr); }
	}
}

bool Map::check_cell_collision(shape::Collider& collider, bool foreground) {
	auto& grid = foreground ? get_obscuring_layer()->grid : get_middleground()->grid;
	auto& layers = m_services->data.get_layers(room_id);
	auto top = get_index_at_position(collider.get_vicinity_rect().position);
	auto bottom = get_index_at_position(collider.get_vicinity_rect().position + collider.get_vicinity_rect().size);
	auto right = static_cast<std::size_t>(collider.get_vicinity_rect().size.x / constants::f_cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<std::size_t>(dimensions.x)) {
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
	auto& grid = get_middleground()->grid;
	auto& layers = m_services->data.get_layers(room_id);
	auto top = get_index_at_position(collider.get_vicinity_rect().position);
	auto bottom = get_index_at_position(collider.get_vicinity_rect().position + collider.get_vicinity_rect().size);
	auto right = static_cast<std::size_t>(collider.get_vicinity_rect().size.x / constants::f_cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<std::size_t>(dimensions.x)) {
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

sf::Vector2i Map::get_circle_collision_result(shape::CircleCollider& collider, bool collide_with_platforms) {
	auto& grid = get_middleground()->grid;
	auto& layers = m_services->data.get_layers(room_id);
	auto top = get_index_at_position(collider.get_vicinity_rect().position);
	auto bottom = get_index_at_position(collider.get_vicinity_rect().position + collider.get_vicinity_rect().size);
	auto right = static_cast<std::size_t>(collider.get_vicinity_rect().size.x / constants::f_cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<std::size_t>(dimensions.x)) {
		auto left{0};
		for (auto j{left}; j <= right; ++j) {
			auto index = i + j;
			if (index >= dimensions.x * dimensions.y || index < 0) { continue; }
			auto& cell = grid.get_cell(static_cast<int>(index));
			if (!cell.is_collidable() || cell.is_ceiling_ramp()) { continue; }
			if (cell.is_platform() && !collide_with_platforms) { continue; }
			cell.collision_check = true;
			if (collider.collides_with(cell.bounding_box)) { return collider.get_collision_result(cell.bounding_box); }
		}
	}
	return {};
}

void Map::handle_cell_collision(shape::CircleCollider& collider) {
	auto& grid = get_middleground()->grid;
	auto top = get_index_at_position(collider.get_vicinity_rect().position);
	auto bottom = get_index_at_position(collider.get_vicinity_rect().position + collider.get_vicinity_rect().size);
	auto right = static_cast<std::size_t>(collider.get_vicinity_rect().size.x / constants::f_cell_size);
	for (auto i{top}; i <= bottom; i += static_cast<std::size_t>(dimensions.x)) {
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

void Map::handle_breakable_collision(shape::CircleCollider& collider) {
	for (auto& b : breakables) { collider.handle_collision(b->get_bounding_box()); }
}

void Map::clear_projectiles() {
	for (auto& proj : active_projectiles) { proj.destroy(false); }
}

void Map::shake_camera() { flags.state.set(LevelState::camera_shake); }

void Map::clear() {
	dimensions = {};
	m_chunks.clear();
	m_colliders.clear();
	// portals.clear();
	platforms.clear();
	breakables.clear();
	pushables.clear();
	spikes.clear();
	destructibles.clear();
	switch_blocks.clear();
	switch_buttons.clear();
	chests.clear();
	// npcs.clear();
	checkpoints.clear();
}

void Map::wrap(sf::Vector2f& position) const {
	if (position.x < 0.f) { position.x = real_dimensions.x; }
	if (position.y < 0.f) { position.y = real_dimensions.y; }
	if (position.x > real_dimensions.x) { position.x = 0.f; }
	if (position.y > real_dimensions.y) { position.y = 0.f; }
}

std::vector<std::unique_ptr<world::Layer>>& Map::get_layers() { return m_services->data.get_layers(room_id); }

std::unique_ptr<world::Layer>& Map::get_middleground() { return m_services->data.get_layers(room_id).at(m_middleground); }

std::unique_ptr<world::Layer>& Map::get_obscuring_layer() { return m_services->data.get_layers(room_id).at(static_cast<std::size_t>(m_services->data.get_layers(room_id).size() - 1)); }

sf::Vector2f Map::get_nearest_target_point(sf::Vector2f from) {
	auto ret = sf::Vector2f{};
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

sf::Vector2f Map::last_checkpoint() {
	for (auto& checkpoint : checkpoints) {
		if (checkpoint.reached()) { return checkpoint.position(); }
	}
	return {};
}

void Map::debug() {
	for (auto& enemy : enemy_catalog.enemies) { enemy->debug(); }
#if defined(FORNANI_PRODUCTION)
	background->debug();
	// for (auto& atm : atmosphere) { atm.debug(); }
#endif
}

bool Map::nearby(shape::Shape& first, shape::Shape& second) const {
	return abs(first.get_position().x + first.get_dimensions().x * 0.5f - second.get_position().x) < constants::f_cell_size * collision_barrier &&
		   abs(first.get_position().y - second.get_position().y) < constants::f_cell_size * collision_barrier;
}

bool Map::within_bounds(sf::Vector2f test) const { return test.x > 0.f && test.x < real_dimensions.x && test.y > 0.f && test.y < real_dimensions.y; }

bool Map::overlaps_middleground(shape::Shape& test) {
	for (auto& cell : get_middleground()->grid.cells) {
		if (test.overlaps(cell.bounding_box) && cell.is_solid()) { return true; }
	}
	return false;
}

dj::Json const& Map::get_json_data(automa::ServiceProvider& svc) const {
	auto rid = room_id;
	auto const it = std::find_if(svc.data.map_jsons.begin(), svc.data.map_jsons.end(), [rid](auto const& r) { return r.id == rid; });
	return it->metadata;
}

std::size_t Map::get_index_at_position(sf::Vector2f position) { return get_middleground()->grid.get_index_at_position(position); }

int Map::get_tile_value_at_position(sf::Vector2f position) { return get_middleground()->grid.get_cell(get_index_at_position(position)).value; }

Tile& Map::get_cell_at_position(sf::Vector2f position) { return get_middleground()->grid.cells.at(get_index_at_position(position)); }

MapAttributes::MapAttributes(dj::Json const& in) {
	// map properties
	if (in["properties"]["environmental_randomness"].as_bool()) { properties.set(MapProperties::environmental_randomness); }
	if (in["properties"]["day_night_shift"].as_bool()) { properties.set(MapProperties::day_night_shift); }
	if (in["properties"]["timer"].as_bool()) { properties.set(MapProperties::timer); }
	if (in["properties"]["lighting"].as_bool()) { properties.set(MapProperties::lighting); }
	if (in["minimap"].as_bool()) { properties.set(MapProperties::minimap); }

	music = in["music"].as_string();
	ambience = in["ambience"].as_string();
	for (auto& entry : in["atmosphere"].as_array()) { atmosphere.push_back(entry.as<int>()); }

	special_drop_id = in["special_drop_id"].as<int>();
	border_color = sf::Color{in["border_color"][0].as<std::uint8_t>(), in["border_color"][1].as<std::uint8_t>(), in["border_color"][2].as<std::uint8_t>()};
}

void MapAttributes::serialize(dj::Json& out) {
	out["properties"]["environmental_randomness"] = properties.test(fornani::world::MapProperties::environmental_randomness);
	out["properties"]["day_night_shift"] = properties.test(fornani::world::MapProperties::day_night_shift);
	out["properties"]["timer"] = properties.test(fornani::world::MapProperties::timer);
	out["properties"]["lighting"] = properties.test(fornani::world::MapProperties::lighting);

	out["music"] = music;
	NANI_LOG_DEBUG(m_logger, "Serialized music: {}", music);
	out["ambience"] = ambience;
	for (auto& atmo : atmosphere) { out["atmosphere"].push_back(atmo); }

	out["special_drop_id"] = special_drop_id;
	out["border_color"].push_back(border_color.r);
	out["border_color"].push_back(border_color.g);
	out["border_color"].push_back(border_color.b);
}

} // namespace fornani::world
