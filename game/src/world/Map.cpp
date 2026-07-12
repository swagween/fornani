
#include <imgui.h>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <djson/json.hpp>
#include <fornani/audio/Balance.hpp>
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/entities/atmosphere/Atmosphere.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/enemy/EnemyRegistry.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/world/Bed.hpp>
#include <fornani/entities/world/Chest.hpp>
#include <fornani/entities/world/Explosion.hpp>
#include <fornani/entities/world/Fire.hpp>
#include <fornani/entities/world/Laser.hpp>
#include <fornani/entities/world/Waterfall.hpp>
#include <fornani/entity/AmbientProp.hpp>
#include <fornani/entity/CutsceneTrigger.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/entity/EntitySet.hpp>
#include <fornani/entity/NPC.hpp>
#include <fornani/entity/Portal.hpp>
#include <fornani/entity/SavePoint.hpp>
#include <fornani/entity/Train.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/entity/Vine.hpp>
#include <fornani/entity/Water.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/graphics/Colors.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/graphics/Scenery.hpp>
#include <fornani/graphics/Weather.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/particle/Emitter.hpp>
#include <fornani/physics/CircleCollider.hpp>
#include <fornani/physics/Collider.hpp>
#include <fornani/physics/ICollider.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/systems/Register.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/ID.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/utils/QuestCode.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/weapon/Weapon.hpp>
#include <fornani/world/Breakable.hpp>
#include <fornani/world/Checkpoint.hpp>
#include <fornani/world/Incinerite.hpp>
#include <fornani/world/Layer.hpp>
#include <fornani/world/Map.hpp>
#include <fornani/world/Platform.hpp>
#include <fornani/world/Pushable.hpp>
#include <fornani/world/Spawner.hpp>
#include <fornani/world/Spike.hpp>
#include <fornani/world/SwitchBlock.hpp>
#include <fornani/world/SwitchButton.hpp>
#include <fornani/world/Tile.hpp>
#include <fornani/world/TimerBlock.hpp>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fornani::world {

Map::Map(automa::ServiceProvider& svc, player::Player& player) : player(&player), enemy_catalog(svc), m_services(&svc), cooldowns{.fade_obscured{util::Cooldown(128)}, .loading{util::Cooldown(24)}} {}

void Map::load(automa::ServiceProvider& svc, [[maybe_unused]] SceneContext& context, int room_number) {

	svc.current_room = room_number;

	unserialize(svc, room_number);

	auto it = std::find_if(svc.data.map_jsons.begin(), svc.data.map_jsons.end(), [room_number](auto const& r) { return r.id == room_number; });
	if (it == svc.data.map_jsons.end()) { return; }

	auto& metadata = it->metadata;
	auto const& meta = metadata["meta"];
	auto& entities = metadata["entities"];
	auto& hazards = metadata["hazards"];

	if (hazards) { m_hazards.emplace(svc, hazards, sf::Vector2u{real_dimensions}); }

	// process properties
	svc.music_player.load(svc.finder, m_attributes.music);
	svc.music_player.play_looped();
	svc.ambience_player.load(svc.finder, m_attributes.ambience);
	svc.ambience_player.play();
	for (auto const& atmo : m_attributes.atmosphere) { atmosphere.push_back(vfx::Atmosphere(svc, *this, atmo)); }

	if (meta["cutscene_on_entry"]["flag"].as_bool()) {
		auto launch = true;
		auto contingency_list = QuestContingencySet{meta["cutscene_on_entry"]["contingencies"]};
		if (meta["cutscene_on_entry"]["contingencies"] && !svc.quest_table.are_contingencies_met(contingency_list)) { launch = false; }

		if (launch) {
			auto ctype = meta["cutscene_on_entry"]["type"].as<int>();
			auto cid = meta["cutscene_on_entry"]["id"].as<int>();
			auto csource = meta["cutscene_on_entry"]["source"].as<int>();
			auto cutscene = util::QuestKey{ctype, cid, csource};
			svc.quest.process(svc, cutscene);
			svc.events.launch_cutscene_event.dispatch(svc, cid);
		}
	}
	for (auto& pl : entities["lights"].as_array()) {
		point_lights.push_back(PointLight(svc.data.light[pl["label"].as_string()], sf::Vector2f{pl["position"][0].as<float>() + 0.5f, pl["position"][1].as<float>() + 0.5f} * constants::f_cell_size));
	}

	if (meta["camera_effects"]) {
		m_attributes.shake_properties.frequency = meta["camera_effects"]["shake"]["frequency"].as<int>();
		m_attributes.shake_properties.energy = meta["camera_effects"]["shake"]["energy"].as<float>();
		m_attributes.shake_properties.start_time = meta["camera_effects"]["shake"]["start_time"].as<float>();
		m_attributes.shake_properties.dampen_factor = meta["camera_effects"]["shake"]["dampen_factor"].as<int>();
		m_attributes.shake_cooldown = util::Cooldown{meta["camera_effects"]["shake"]["frequency_in_seconds"].as<int>()};
		m_attributes.shake_properties.shaking = m_attributes.shake_properties.frequency > 0;
		m_attributes.shake_cooldown.start();
	}

	sound.echo_count = meta["sound"]["echo_count"].as<int>();
	sound.echo_rate = meta["sound"]["echo_rate"].as<int>();

	if (meta["weather"]) {
		m_weather_specs.emplace(vfx::WeatherSpecifications{meta["weather"]});
		if (svc.world_clock.happens(WorldClockInterval::day, m_weather_specs->chance)) {
			m_weather = std::make_unique<vfx::Weather>(svc, *this, m_weather_specs->params);
			svc.ambience_player.load(svc.finder, m_weather_specs->ambience);
			svc.ambience_player.play();
		}
	}
	auto bg_type = m_weather ? meta["background"].as_string() + "_" + m_weather_specs->type : meta["background"].as_string();
	background = std::make_unique<graphics::Background>(svc, bg_type, real_dimensions);

	// if (meta["weather"]["snow"]) { rain = vfx::Rain(meta["weather"]["snow"]["intensity"].as<int>(), meta["weather"]["snow"]["fall_speed"].as<float>(), meta["weather"]["snow"]["slant"].as<float>(), true); }
	// if (meta["weather"]["leaves"]) { rain = vfx::Rain(meta["weather"]["leaves"]["intensity"].as<int>(), meta["weather"]["leaves"]["fall_speed"].as<float>(), meta["weather"]["leaves"]["slant"].as<float>(), true, true); }

	for (auto& entry : entities["chests"].as_array()) {
		sf::Vector2f pos{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		if (!svc.quest_table.are_contingencies_met({QuestContingencySet{entry["contingencies"]}})) { continue; }
		if (entry["tag"]) {
			chests.push_back(std::make_unique<entity::Chest>(svc, *this, StableID::from(room_id, pos.x, pos.y), static_cast<entity::ChestType>(entry["type"].as<int>()), entry["tag"].as_string(), entry["modifier"].as<int>()));
		} else {
			chests.push_back(std::make_unique<entity::Chest>(svc, *this, StableID::from(room_id, pos.x, pos.y), static_cast<entity::ChestType>(entry["type"].as<int>()), entry["modifier"].as<int>()));
		}
		chests.back()->set_position_from_scaled(pos);
	}

	/*for (auto& entry : entities["animators"].as_array()) {
		sf::Vector2i scaled_pos{};
		scaled_pos.x = entry["position"][0].as<int>();
		scaled_pos.y = entry["position"][1].as<int>();
		animators.push_back(entity::Animator(svc, entry["label"].as_string(), entry["id"].as<int>(), scaled_pos, entry["foreground"].as_bool()));
	}*/

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
	/*for (auto [i, entry] : std::views::enumerate(entities["inspectables"].as_array())) {
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
		if (svc.data.inspectable_is_destroyed(inspectables.back().get_stable_id())) { inspectables.back().destroy(); }
	}*/

	if (m_entities) {
		for (auto [k, i] : std::views::enumerate(get_entities<Inspectable>())) { i->set_index(k); }
	}
	// for (auto& entry : entities["destructibles"].as_array()) { destructibles.push_back(std::make_unique<Destructible>(svc, *this, entry, m_biome.get_id())); }

	for (auto& entry : entities["enemies"].as_array()) {
		int id{};
		sf::Vector2f pos{};
		sf::Vector2<int> start{};
		pos.x = entry["position"][0].as<float>();
		pos.y = entry["position"][1].as<float>();
		start.x = entry["start_direction"][0].as<int>();
		start.y = entry["start_direction"][1].as<int>();
		auto variant = entry["variant"].as<int>();
		auto spawn_amount = 1;
		auto spawn_range = sf::Vector2f{};
		for (auto i = 0; i < spawn_amount; ++i) {
			if (auto id = svc.data.get_enemy_label_from_id(entry["id"].as<int>())) {
				if (svc.data.enemy[*id]["multispawn"]) {
					auto& in = svc.data.enemy[*id]["multispawn"];
					spawn_amount = random::random_range_float(in["quantity"][0].as<int>(), in["quantity"][1].as<int>());
					spawn_range = random::random_vector_float({-in["spread"][0].as<float>(), -in["spread"][1].as<float>()}, {in["spread"][0].as<float>(), in["spread"][1].as<float>()});
				}
			}
			enemy_catalog.push_enemy(svc, *this, context, entry["id"].as<int>(), {variant, start, enemy::Multispawn{spawn_range}, false});
			enemy_catalog.enemies.back()->set_position_from_scaled(sf::Vector2f{pos * constants::f_cell_size});
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			enemy_catalog.enemies.back()->set_stable_id({room_id, {static_cast<int>(pos.x), static_cast<int>(pos.y)}});
			if (svc.data.enemy_is_fallen(room_id, enemy_catalog.enemies.back()->get_stable_id())) { enemy_catalog.enemies.pop_back(); }
		}
	}

	auto plat_handle = EntityHandle{3000};
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
		start = std::clamp(start, 0.f, 1.f);
		auto type = entry["type"].as_string();
		platforms.push_back(std::make_unique<Platform>(svc, *this, pos, dim, entry["extent"].as<float>(), type, start, entry["style"].as<int>()));
		platforms.back()->set_handle(++plat_handle);
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
	cooldowns.loading.start();

	player->register_with_map(*this);
	if (m_biome.get_id() == 12) {
		player->texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_night"));
		player->catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_night"));
		player->update_sprite();
		player->update_wardrobe();
	} else {
		player->texture_updater.load_pixel_map(svc.assets.get_texture_modifiable("nani_palette_default"));
		player->catalog.wardrobe.set_palette(svc.assets.get_texture_modifiable("nani_palette_default"));
		player->update_sprite();
		player->update_wardrobe();
	}
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
	spawn_counter.start();

	svc.state_flags.reset(automa::StateFlags::hide_hud);

	svc.world_timer.set_tag("nani");
	svc.world_timer.set_course(room_number);

	m_biome = svc.data.construct_biome(it->biome_label);
	m_metadata.room = it->room_label;

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

	// check for enemy respawns
	svc.data.respawn_enemies(room_id, player->visit_history.distance_traveled_from(room_id));

	m_middleground = metadata["tile"]["middleground"].as<int>();
	metadata["tile"]["flags"]["obscuring"].as_bool() ? m_layer_properties.set(LayerProperties::has_obscuring_layer) : m_layer_properties.reset(LayerProperties::has_obscuring_layer);
	metadata["tile"]["flags"]["reverse_obscuring"].as_bool() ? m_layer_properties.set(LayerProperties::has_reverse_obscuring_layer) : m_layer_properties.reset(LayerProperties::has_reverse_obscuring_layer);

	m_chunks.resize(static_cast<std::size_t>((dimensions.x / constants::u32_chunk_size) * (dimensions.y / constants::u32_chunk_size)));

	m_attributes.border_color = Color{svc.data.biomes["properties"][m_biome.get_label()]["black"]};
	if (entities.is_object()) { m_entities = EntitySet(svc, *this, svc.finder, entities, m_metadata.room); }

	auto u_dim = sf::Vector2u{real_dimensions};
	if (!m_entity_texture.resize(u_dim)) { NANI_LOG_WARN(m_logger, "Failed to resize entity texture!"); }
	if (!m_static_entity_texture.resize(u_dim)) { NANI_LOG_WARN(m_logger, "Failed to resize static entity texture!"); }
	m_static_entity_texture.clear(colors::transparent);

	generate_collidable_layer(live);
}

void Map::update(automa::ServiceProvider& svc, SceneContext& context) {
	if (!player->has_collider()) { return; }
	auto& layers = svc.data.get_layers(room_id);
	flags.state.reset(LevelState::camera_shake);

	for (auto& layer : get_layers()) { layer->update(svc); }

	update_balance(svc);

	// weather
	if (svc.ticker.every_x_ticks(24)) {
		if (m_weather && !m_attributes.properties.test(MapProperties::interior)) {
			auto chance_per_point = 20;
			for (auto& p : m_surface_points) {
				if (random::percent_chance(chance_per_point)) {
					auto pos = p.position + sf::Vector2f{random::random_range_float(-8.f, 8.f), 0.f};
					spawn_effect(svc, m_weather_specs->effect, pos);
				}
			}
		}
	}

	// camera effects
	if (svc.ticker.every_second() && m_attributes.shake_properties.shaking) {
		if (random::percent_chance(m_attributes.shake_properties.chance * 100.f)) {
			m_attributes.shake_cooldown.update();
			if (m_attributes.shake_cooldown.is_complete()) {
				svc.camera_controller.shake(m_attributes.shake_properties);
				auto shake_time = m_attributes.shake_cooldown.get_native_time();
				auto diff = random::random_range(-shake_time / 2, shake_time / 2);
				m_attributes.shake_cooldown.start(shake_time + diff);
			}
		}
	}

	// enemy spawns
	if (flags.state.test(LevelState::spawn_enemy)) {
		for (auto& spawn : enemy_spawns) {
			enemy_catalog.push_enemy(*m_services, *this, context, spawn.id, {spawn.variant, {-1, 0}, {}, true});
			enemy_catalog.enemies.back()->intangible_start(64);
			enemy_catalog.enemies.back()->set_position(spawn.pos - enemy_catalog.enemies.back()->get_collider().get_local_center());
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			if (spawn.effect) { spawn_effect(svc, "small_flash", enemy_catalog.enemies.back()->get_collider().get_center()); }
		}
		enemy_spawns.clear();
		flags.state.reset(LevelState::spawn_enemy);
	}

	// chain explosions
	std::erase_if(m_chain_explosions, [](auto const& e) { return e.volatility == 0; });
	for (auto& explosion : m_chain_explosions) {
		if (explosion.delay.is_almost_complete()) {
			explosion.volatility = 0;
			spawn_explosion(svc, explosion.tag, explosion.emitter, explosion.team, explosion.position + random::random_vector_float(-80.f, 80.f), explosion.radius, explosion.channel);
		}
		explosion.delay.update();
	}

	// hidden areas
	flags.map_state.test(MapState::unobscure) ? cooldowns.fade_obscured.update() : cooldowns.fade_obscured.reverse();
	if (check_cell_collision(player->get_collider(), true)) {
		if (!flags.map_state.test(MapState::unobscure)) {
			cooldowns.fade_obscured.start();
			if (has_obscuring_layer()) { svc.soundboard.play_sound("reveal_" + std::string{m_biome.get_label()}); }
		}
		if (cooldowns.loading.running()) { cooldowns.fade_obscured.cancel(); }
		flags.map_state.set(MapState::unobscure);
	} else {
		if (flags.map_state.test(MapState::unobscure)) { cooldowns.fade_obscured.cancel(); }
		flags.map_state.reset(MapState::unobscure);
	}

	// hazards
	if (m_hazards) { m_hazards->update(*player, *this, context.transition); }

	std::erase_if(active_emitters, [](auto const& p) { return p->done(); });
	std::erase_if(effects, [](auto& e) { return e.done(); });
	std::erase_if(lasers, [](auto const& l) { return l.is_complete(); });
	std::erase_if(incinerite_blocks, [](auto const& i) { return i->is_destroyed(); });
	std::erase_if(breakables, [](auto const& b) { return b->is_destroyed(); });
	std::erase_if(m_explosions, [](auto const& e) { return e.is_done(); });
	enemy_catalog.update();

	manage_projectiles(svc);
	for (auto& proj : active_projectiles) {
		if (proj.destruction_initiated()) { continue; }
		proj.register_chunk(get_chunk_id_from_position(proj.get_position()));
		for (auto& platform : platforms) { platform->on_hit(svc, *this, proj); }
		for (auto& breakable : breakables) { breakable->on_hit(svc, *this, proj); }
		for (auto& pushable : pushables) { pushable->on_hit(svc, *this, proj); }
		for (auto destructible : get_entities<Destructible>()) { destructible->on_hit(svc, *this, proj); }
		for (auto& block : switch_blocks) { block->on_hit(svc, *this, proj); }
		for (auto& enemy : enemy_catalog.enemies) { enemy->on_hit(svc, *this, proj, *player); }
		for (auto& incinerite : incinerite_blocks) { incinerite->on_hit(svc, *this, proj); }
		for (auto vine : get_entities<Vine>()) { vine->on_hit(svc, *this, proj, *player); }
		proj.handle_collision(svc, *this);
		proj.on_player_hit(svc, *this, *player);
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
		for (auto chunk : collider.get_chunks()) {
			for (auto& other_ptr : m_chunks[chunk]) {
				if (!other_ptr) { continue; }
				if (other_ptr == &collider) { continue; }
				if (collider.is_intangible()) { continue; }
				collider.handle_collision(*other_ptr);
				++num_collision_checks;
			}
		}
		if (!collider.has_attribute(shape::ColliderAttributes::fixed)) { collider.detect_map_collision(*this); }
	}

	for (auto& enemy : enemy_catalog.enemies) { enemy->post_update(svc, *this, *player); }
	for (auto& pushable : pushables) { pushable->update(svc, *this, *player); }
	for (auto& platform : platforms) { platform->post_update(svc, *this, *player); }

	player->on_crush(*this);
	player->reset_water_flags();

	if (m_entities) {
		for (auto& entity : m_entities.value().variables.entities) { entity->update(svc, *this, context, *player); }
	}
	for (auto& f : fire) { f.update(svc, *player, *this, context.console); }

	for (auto& laser : lasers) { laser.update(svc, *player, *this); }
	for (auto& exp : m_explosions) { exp.update(svc, *player, *this); }
	for (auto& loot : active_loot) { loot.update(svc, *this, *player); }
	for (auto& chest : chests) { chest->update(svc, *this, context.console, *player); }
	// for (auto& inspectable : inspectables) { inspectable.update(svc, *this, context, *player); }
	// for (auto& animator : animators) { animator.update(); }
	for (auto& effect : effects) { effect.update(); }
	for (auto& atm : atmosphere) { atm.update(svc, *this, *player); }
	for (auto& spawner : spawners) { spawner.update(svc, *this); }
	for (auto& switch_block : switch_blocks) { switch_block->update(svc, *this, *player); }
	for (auto& switch_button : switch_buttons) { switch_button->update(svc, *this, *player); }
	// for (auto& destructible : destructibles) { destructible->update(svc, *this, *player); }
	for (auto& checkpoint : checkpoints) { checkpoint.update(svc, *this, *player); }
	for (auto& bed : beds) { bed.update(svc, *this, context, *player); }
	for (auto& breakable : breakables) { breakable->update(svc, *this, *player); }
	for (auto& waterfall : waterfalls) { waterfall->update(svc, *this, *player); }
	for (auto& incinerite : incinerite_blocks) { incinerite->update(svc, *this, *player); }
	if (test_mobile) { test_mobile->update(*this); }

	for (auto& pushable : pushables) { pushable->post_update(svc, *this, *player); }
	for (auto& spike : spikes) { spike.update(svc, *player, *this); }
	// for (auto& vine : vines) { vine->update(svc, *this, *player); }
	for (auto& timer_block : timer_blocks) { timer_block.update(svc, *this, *player); }
	for (auto& pl : point_lights) { pl.update(); }
	if (player->get_collider().collision_depths) { player->get_collider().collision_depths.value().update(); }
	// if (save_point) { save_point->update(svc, *player, console); }
	if (m_weather) { m_weather.value()->update(svc, *this); }

	player->get_collider().reset_ground_flags();

	cooldowns.loading.update();
}

void Map::render(Renderer& renderer, automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam) {

	m_entity_texture.clear(colors::transparent);

	auto use_shader = m_attributes.properties.test(MapProperties::lighting) && m_palette && shader;
	if (use_shader) { shader->finalize(svc.data.biomes["properties"][get_biome_string()]["max_light"].as<float>()); }

	if (m_entities) {
		// TODO: uncomment below once all entities have been refactored!
		// for (auto& entity : m_entities.value().variables.entities) { entity->render(win, cam, 1.0); }
		for (auto p : get_entities<Portal>()) {
			if (p->has_custom_animation() || has_property(MapProperties::lighting)) {
				p->render(svc, m_entity_texture, cam);
			} else {
				p->render(win, cam, 1.f);
			}
		}
		for (auto a : get_entities<Animator>()) {
			if (!a->is_foreground()) { a->render(m_entity_texture, cam); }
		}
		for (auto s : get_entities<SavePoint>()) { s->submit(renderer); }
		renderer.flush();

		if (svc.greyblock_mode()) {
			for (auto c : get_entities<CutsceneTrigger>()) { c->render(win, cam, c->get_f_grid_dimensions().x); }
		}
		for (auto v : get_entities<Vine>()) {
			if (!v->is_foreground()) { svc.greyblock_mode() ? v->render(win, cam, 1.f) : v->submit(renderer); }
		}
		renderer.flush();

		for (auto i : get_entities<Inspectable>()) { i->render(win, cam, 1.f); }
		// for (auto n : get_entities<NPC>()) { n->render(win, cam, 1.0); }
	}

	// for (auto& portal : portals) { portal.render(svc, win, cam); }
	for (auto& f : fire) { f.submit(renderer); }
	renderer.flush();
	for (auto& bed : beds) { bed.render(svc, win, cam); }
	for (auto& chest : chests) { chest->render(win, cam); }

	if (m_entities) {
		for (auto n : get_entities<NPC>()) {
			if (!n->is_background()) { n->render(win, cam); }
		}
	}

	for (auto& enemy : enemy_catalog.enemies) {
		if (enemy->is_background()) { enemy->render(svc, win, cam); }
	}
	player->render(svc, win, cam);
	for (auto& enemy : enemy_catalog.enemies) {
		if (!enemy->is_foreground() && !enemy->is_background()) { enemy->render(svc, win, cam); }
	}
	for (auto& proj : active_projectiles) { proj.render(svc, *player, win, cam); }
	for (auto& loot : active_loot) { loot.render(svc, win, cam); }
	for (auto& emitter : active_emitters) { svc.greyblock_mode() ? emitter->render(svc, win, cam) : emitter->submit(renderer); }
	renderer.flush();
	for (auto& plat : platforms) { has_property(MapProperties::lighting) ? plat->render(svc, m_entity_texture, cam) : plat->render(svc, win, cam); }
	for (auto& breakable : breakables) { breakable->render(svc, win, cam); }
	for (auto& incinerite : incinerite_blocks) { incinerite->render(svc, win, cam); }
	for (auto& pushable : pushables) { pushable->render(svc, win, cam); }
	for (auto& checkpoint : checkpoints) { checkpoint.render(svc, win, cam); }
	for (auto& switch_block : switch_blocks) { switch_block->render(svc, win, cam); }
	for (auto& switch_button : switch_buttons) { switch_button->render(svc, win, cam); }
	for (auto& atm : atmosphere) { atm.render(renderer); }
	renderer.flush();
	for (auto& exp : m_explosions) { exp.render(svc, win, cam); }
	if (!has_property(MapProperties::lighting)) {
		for (auto& spike : spikes) { spike.render(svc, win, shader, m_palette, cam); }
	}

	if (svc.greyblock_mode()) {
		sf::CircleShape c{4.f};
		c.setFillColor(colors::orange);
		for (auto& point : m_surface_points) {
			c.setPosition(point.position - cam);
			win.draw(c);
		}
	}

	if (m_entities) {
		for (auto w : get_entities<Water>()) { w->render(win, cam, 1.0); }
	}

	if (!svc.greyblock_mode()) {
		for (auto [i, layer] : std::views::enumerate(get_layers())) {
			if (use_shader && !layer->ignore_lighting()) {
				shader->set_texture_size(real_dimensions / constants::f_scale_factor);
				shader->finalize(svc.data.biomes["properties"][get_biome_string()]["max_light"].as<float>());
				layer->render(svc, win, shader.value(), m_palette.value(), m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, false, m_attributes.properties.test(MapProperties::day_night_shift));
			} else {
				layer->render(svc, win, m_camera_effects.shifter, cooldowns.fade_obscured.get_normalized(), cam, false, m_attributes.properties.test(MapProperties::day_night_shift));
			}
			if (i == m_middleground) {
				for (auto n : get_entities<AmbientProp>()) {
					if (n->is_foreground()) { n->render(win, cam, 1.f); }
				}
			}
		}
	}

	// for (auto& destructible : destructibles) { destructible->render(svc, win, cam); }
	for (auto d : get_entities<Destructible>()) { d->is_unlit() ? d->render(win, cam, 1.f) : d->render(m_entity_texture, cam); }

	if (m_entities) {
		for (auto v : get_entities<Vine>()) {
			if (v->is_foreground()) { v->submit(renderer); }
		}
		renderer.flush();
		for (auto t : get_entities<Turret>()) { t->render(win, cam, 1.0); }
	}

	if (test_mobile) { test_mobile->render(win, cam); }

	// foreground enemies
	for (auto& enemy : enemy_catalog.enemies) {
		if (enemy->is_foreground()) { enemy->render(svc, win, cam); }
		enemy->render_indicators(svc, win, cam);
		enemy->gui_render(svc, win, cam);
	}

	for (auto& laser : lasers) { laser.render(svc, win, cam); }
	if (!svc.greyblock_mode()) {
		for (auto& effect : effects) { effect.submit(renderer); }
	}
	renderer.flush();

	player->render_indicators(svc, win, cam);
	if (m_entities) {
		for (auto a : get_entities<Animator>()) {
			if (a->is_foreground()) { a->render(m_entity_texture, cam); }
		}
	}
	/*for (auto& animator : animators) {
		if (animator.is_foreground()) { animator.render(win, cam); }
	}*/

	// for (auto& inspectable : inspectables) { inspectable.render(svc, win, cam); }

	if (m_weather && !m_attributes.properties.test(MapProperties::interior)) { m_weather.value()->render(svc, win, cam, 0); }

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

	m_entity_texture.display();
}

void Map::render_background(Renderer& renderer, automa::ServiceProvider& svc, sf::RenderWindow& win, std::optional<LightShader>& shader, sf::Vector2f cam) {

	if (!svc.greyblock_mode()) {
		background->render(svc, win, cam);
		if (m_weather) { m_weather.value()->render(svc, win, cam, 2); }
		for (auto& layer : scenery_layers) {
			for (auto& piece : layer) { piece->render(svc, win, cam); }
		}
		if (!svc.greyblock_mode()) {
			for (auto [i, layer] : std::views::enumerate(get_layers())) {
				if (i == 1) {
					if (m_weather && !m_attributes.properties.test(MapProperties::interior)) { m_weather.value()->render(svc, win, cam, 1); }
					if (m_entities) {
						for (auto n : get_entities<AmbientProp>()) {
							if (!n->is_foreground()) { n->render(win, cam, 1.f); }
						}
						for (auto n : get_entities<NPC>()) {
							if (n->is_background()) { n->render(win, cam); }
						}
						for (auto t : get_entities<Train>()) { t->render(win, cam, 1.f); }
					}
				}
				if (m_attributes.properties.test(MapProperties::lighting) && m_palette && shader && !layer->ignore_lighting()) {
					shader->finalize(svc.data.biomes["properties"][get_biome_string()]["max_light"].as<float>());
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
	/*for (auto& animator : animators) {
		if (!animator.is_foreground()) { animator.render(win, cam); }
	}*/
	for (auto& timer_block : timer_blocks) { timer_block.render(svc, win, cam); }
	for (auto& waterfall : waterfalls) { waterfall->render(svc, win, cam); }

	if (m_hazards) { m_hazards->render(win, cam); }
}

bool Map::handle_entry(player::Player& player, util::Cooldown& enter_room) {
	auto ret = false;
	if (!m_entities) { return false; }
	for (auto const& portal : get_entities<Portal>()) {
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
			if (portal->is_bottom()) {
				player.get_collider().physics.acceleration.y = -player.physics_stats.jump_velocity;
				player.accumulated_forces.push_back(sf::Vector2f{player::walk_speed_v * player.get_actual_direction().as_float(), 0.f});
			}
		}
	}
	return ret;
}

void Map::clear_weather() {
	m_weather.reset();
	m_weather_specs.reset();

	auto const& metadata = m_services->data.get_map_json_from_id(room_id);
	if (!metadata) { return; }
	auto const bg_type = metadata.value().get()["meta"]["background"].as_string();
	background = std::make_unique<graphics::Background>(*m_services, bg_type, real_dimensions);
}

void Map::spawn_laser(automa::ServiceProvider& svc, Turret& parent, sf::Vector2f position, LaserType type, util::BitFlags<LaserAttributes> attributes, CardinalDirection direction, int active, int cooldown, float size) {
	lasers.push_back(Laser(svc, *this, parent, position, type, attributes, direction, active, cooldown, size));
}

void Map::spawn_laser(automa::ServiceProvider& svc, sf::Vector2f position, CardinalDirection direction, arms::LaserSpecifications specs) {
	lasers.push_back(Laser(svc, *this, position, specs.type, specs.attributes, direction, specs.active, specs.cooldown, specs.size));
	lasers.back().set_damage(specs.damage);
}

auto Map::get_chunk_id_from_position(sf::Vector2f pos) const -> std::uint8_t {
	auto clamped = sf::Vector2f{std::clamp(pos.x, 0.f, real_dimensions.x), std::clamp(pos.y, 0.f, real_dimensions.y)};
	auto clookup = (clamped / constants::f_cell_size) / constants::f_chunk_size;
	auto ulookup = sf::Vector2u{clookup};
	auto ret = std::clamp(ulookup.y * get_chunk_dimensions().x + ulookup.x, 0u, static_cast<unsigned int>(m_chunks.size() - 1));
	return static_cast<std::uint8_t>(ret);
}

void Map::spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2f pos, sf::Vector2f target, float speed_multiplier, float damage_multiplier) {
	active_projectiles.push_back(weapon.projectile);
	active_projectiles.back().set_position(pos);
	active_projectiles.back().seed(svc, target, speed_multiplier, damage_multiplier);
	active_projectiles.back().update(svc, *player);
	active_projectiles.back().register_chunk(get_chunk_id_from_position(pos));

	if (weapon.secondary_emitter) { spawn_emitter(svc, weapon.secondary_emitter.value().type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().color); }
	spawn_emitter(svc, weapon.emitter.type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.emitter.dimensions, weapon.emitter.color);
	if (weapon.secondary_emitter) { spawn_emitter(svc, weapon.secondary_emitter.value().type, weapon.get_barrel_point(), weapon.get_firing_direction(), weapon.secondary_emitter.value().dimensions, weapon.secondary_emitter.value().color); }
}

void Map::spawn_effect(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2f pos, sf::Vector2f vel, int channel, float scale) { effects.push_back(entity::Effect(svc, tag.data(), pos, vel, channel, scale)); }

void Map::spawn_emitter(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2f pos, Direction dir, sf::Vector2f dim, sf::Color color, int channel) {
	active_emitters.push_back(std::make_unique<vfx::Emitter>(svc, *this, pos, dim, tag, color, dir, channel));
}

void Map::spawn_explosion(automa::ServiceProvider& svc, std::string_view tag, std::string_view emitter, arms::Team team, sf::Vector2f pos, float radius, int channel, int volatility, bool stun) {
	m_explosions.push_back(Explosion{svc, team, pos, radius, stun});
	spawn_effect(svc, tag, pos, {}, channel);
	spawn_emitter(svc, emitter, pos, Direction{});
	if (volatility > 0) {
		for (auto i = 0; i < volatility; ++i) {
			m_chain_explosions.push_back(ExplosionSpecifications{tag.data(), emitter.data(), team, pos, radius, channel, 1});
			m_chain_explosions.back().delay.start(i * random::random_range(24, 48));
		}
	}
}

void Map::spawn_enemy(int id, sf::Vector2f pos, int variant, bool allow_proximity_to_player, bool effect) {
	if (!allow_proximity_to_player) {
		auto break_out = 0;
		while (player->distant_vicinity.contains_point(pos) && break_out < 32) {
			auto distance = player->get_collider().get_center() - pos;
			pos -= distance;
			++break_out;
		}
	}
	auto break_out = 0;
	while ((overlaps_middleground(pos) || !within_bounds(pos)) && break_out < 32) {
		auto distance = (pos - real_dimensions * 0.5f) * 0.1f;
		pos += distance;
		++break_out;
	}
	enemy_spawns.push_back({pos, id, variant, effect});
	spawn_counter.update();
	flags.state.set(LevelState::spawn_enemy);
}

void Map::spawn_chest(automa::ServiceProvider& svc, enemy::Treasure const& treasure, sf::Vector2f pos, sf::Vector2f vel) {
	switch (treasure.type) {
	case entity::ChestType::item: chests.push_back(std::make_unique<entity::Chest>(svc, *this, StableID{}, entity::ChestType::item, treasure.tag, svc.data.item_id_from_label(treasure.tag))); break;
	case entity::ChestType::gun: chests.push_back(std::make_unique<entity::Chest>(svc, *this, StableID{}, entity::ChestType::gun, treasure.tag, svc.data.get_gun_id_from_tag(treasure.tag))); break;
	case entity::ChestType::orbs: chests.push_back(std::make_unique<entity::Chest>(svc, *this, StableID{}, entity::ChestType::orbs, 0)); break;
	default: break;
	}
	chests.back()->set_position(pos);
	chests.back()->get_collider().physics.velocity = vel;
	chests.back()->set_attribute(entity::ChestAttributes::mythic, treasure.mythic);
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
			effects.push_back(entity::Effect(svc, "bullet_whiff", proj.get_collider().get_global_center(), proj.get_velocity() * 0.05f, proj.effect_type()));
			proj.poof();
		}
	}

	std::erase_if(active_projectiles, [](auto const& p) { return p.destroyed(); });
}

void Map::generate_collidable_layer(bool live) {
	auto train_style = get_style_id() == 1 ? 1 : 0;
	auto pushable_offset = sf::Vector2f{1.f, 0.f};
	for (auto& cell : get_middleground()->grid.cells) {
		auto chunk_id = cell.get_chunk_id();
		get_middleground()->grid.check_neighbors(cell.one_d_index);
		if (live) { continue; }
		if (cell.is_waterfall()) { waterfalls.push_back(std::make_unique<Waterfall>(*m_services, *this, sf::Vector2u{cell.f_scaled_position()})); }
		if (cell.is_breakable()) { breakables.push_back(std::make_unique<Breakable>(*m_services, *this, cell.position())); }
		if (cell.is_pushable()) { pushables.push_back(std::make_unique<Pushable>(*m_services, *this, cell.position() + pushable_offset, get_style_id(), cell.value - 483)); }
		if (cell.is_spike()) {
			spikes.push_back(Spike(*m_services, m_services->assets.get_tileset(std::string{get_biome_string()}), cell.position(), get_middleground()->grid.get_solid_neighbors(cell.one_d_index), m_biome.get_id(),
								   m_attributes.properties.test(MapProperties::environmental_randomness)));
			m_static_entity_texture.draw(spikes.back());
		}
		if (cell.is_spawner()) { spawners.push_back(Spawner(*m_services, cell.get_global_center(), 5)); }
		if (cell.is_target()) { target_points.push_back(cell.get_global_center()); }
		if (cell.is_home()) { home_points.push_back(cell.get_global_center()); }
		if (cell.is_incinerite()) { incinerite_blocks.push_back(std::make_unique<Incinerite>(*m_services, *this, cell.position(), chunk_id)); }
		if (cell.is_checkpoint()) { checkpoints.push_back(Checkpoint(*m_services, cell.position())); }
		if (cell.is_fire()) { fire.push_back(Fire(*m_services, cell.position(), cell.value)); }
		if (cell.is_solid() && get_middleground()->grid.is_exposed_to_sky(cell.one_d_index, m_attributes.sky_limit)) { m_surface_points.push_back(SurfacePoint{cell.bounding_box.get_top(), true}); }
	}
	m_static_entity_texture.display();
}

void Map::generate_layer_textures(automa::ServiceProvider& svc) const {
	for (auto& layer : svc.data.get_layers(room_id)) { layer->generate_textures(svc.assets.get_tileset(std::string{get_biome_string()}), m_attributes.properties.test(MapProperties::day_night_shift)); }
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

shape::Collider* Map::create_collider(sf::Vector2f dim) {
	auto ptr = std::make_unique<shape::Collider>(dim);
	auto raw = ptr.get();
	m_colliders.push_back(std::move(ptr));
	return raw;
}

shape::CircleCollider* Map::create_collider(float radius) {
	auto ptr = std::make_unique<shape::CircleCollider>(radius);
	auto raw = ptr.get();
	m_colliders.push_back(std::move(ptr));
	return raw;
}

void Map::destroy_collider(shape::ICollider* collider) {
	// remove from chunks first
	for (auto& chunk : m_chunks) { std::erase(chunk, collider); }
	// destroy ownership
	std::erase_if(m_colliders, [&](std::unique_ptr<shape::ICollider> const& ptr) { return ptr.get() == collider; });
}

void Map::refresh_collider_chunks(Register<int> const& old_chunks, Register<int> const& new_chunks, shape::ICollider* ptr) {
	for (int chunk : old_chunks) {
		if (!new_chunks.contains(chunk)) {
			if (chunk < m_chunks.size()) {
				auto& bucket = m_chunks[chunk];
				bucket.erase(std::remove(bucket.begin(), bucket.end(), ptr), bucket.end());
			}
		}
	}

	for (int chunk : new_chunks) {
		if (!old_chunks.contains(chunk)) {
			auto& bucket = m_chunks[chunk];

			if (std::find(bucket.begin(), bucket.end(), ptr) == bucket.end()) bucket.push_back(ptr);
		}
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
			if (cell.is_platform() && (world::is_above_platform(cell, collider.get_radius() + collider.get_global_center().y) || collider.physics.actual_velocity().y < 0.f)) { continue; }
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

void Map::clear_enemies(std::unordered_set<int> const exceptions) {
	for (auto& enemy : enemy_catalog.enemies) {
		if (!exceptions.contains(enemy->get_id())) {
			enemy->kill();
			NANI_LOG_DEBUG(m_logger, "Killed enemy {}", enemy->get_id());
		}
	}
}

void Map::shake_camera() { flags.state.set(LevelState::camera_shake); }

void Map::clear() {
	m_entities.reset();
	platforms.clear();
	breakables.clear();
	pushables.clear();
	// destructibles.clear();
	switch_blocks.clear();
	switch_buttons.clear();
	incinerite_blocks.clear();
	chests.clear();
	checkpoints.clear();
	atmosphere.clear();
	enemy_catalog.enemies.clear();
	active_emitters.clear();
	beds.clear();
	background.reset();
	active_projectiles.clear();
	// inspectables.clear();
	// animators.clear();
	effects.clear();
	for (auto& scenery : scenery_layers) { scenery.clear(); }
	lasers.clear();
	spawners.clear();
	spikes.clear();
	timer_blocks.clear();
	enemy_spawns.clear();
	target_points.clear();
	home_points.clear();
	waterfalls.clear();
	m_explosions.clear();
	m_chain_explosions.clear();
	m_weather.reset();
	m_weather_specs.reset();
	fire.clear();
	active_loot.clear();
	m_hazards.reset();
	m_surface_points.clear();
	point_lights.clear();
}

void Map::wrap(sf::Vector2f& position) const {
	if (position.x < 0.f) { position.x = real_dimensions.x; }
	if (position.y < 0.f) { position.y = real_dimensions.y; }
	if (position.x > real_dimensions.x) { position.x = 0.f; }
	if (position.y > real_dimensions.y) { position.y = 0.f; }
}

void Map::set_target_balance(float const to, audio::BalanceTarget const target) {
	switch (target) {
	case audio::BalanceTarget::music: music_balance.set_target(to); break;
	case audio::BalanceTarget::ambience: ambience_balance.set_target(to); break;
	default: break;
	}
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

sf::Vector2f Map::get_switch_block_position(int id) const {
	auto running = sf::Vector2f{};
	auto counted = 0;
	for (auto const& block : switch_blocks) {
		if (block->get_id() == id) {
			running += block->get_bounding_box().get_center();
			++counted;
		}
	}
	if (counted == 0) { return running; }
	return running / static_cast<float>(counted);
}

sf::Vector2f Map::last_checkpoint() {
	for (auto& checkpoint : checkpoints) {
		if (checkpoint.reached()) { return checkpoint.position(); }
	}
	return {};
}

void Map::debug() {
	ImGui::Text("Biome: %s", std::string{m_biome.get_label()}.c_str());
	static float m = 1.f;
	static float a = 1.f;
	ImGui::SliderFloat("Music Balance", &m, 0.f, 1.f);
	ImGui::SliderFloat("Ambience Balance", &a, 0.f, 1.f);
	music_balance.set_target(m);
	ambience_balance.set_target(a);
	ImGui::Separator();
	if (ImGui::Button("Test Collider")) {
		if (test_mobile) {
			test_mobile.reset();
		} else {
			test_mobile.emplace(*m_services, *this);
		}
	}
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

bool Map::overlaps_middleground(sf::Vector2f test) {
	for (auto& cell : get_middleground()->grid.cells) {
		if (cell.bounding_box.contains_point(test) && cell.is_solid()) { return true; }
	}
	return false;
}

sf::Vector2f Map::compute_mtv(sf::Vector2f test) {
	for (auto& cell : get_middleground()->grid.cells) {
		if (cell.bounding_box.contains_point(test) && cell.is_solid()) { return cell.bounding_box.compute_mtv(test); }
	}
	return {};
}

auto Map::get_music_balance() const -> float { return music_balance.get(); }

auto Map::get_closest_home_point(sf::Vector2f const check) const -> sf::Vector2f {
	auto ret = sf::Vector2f{};
	auto closest = std::numeric_limits<float>::max();
	for (auto& pt : home_points) {
		auto distance = (check - pt).length();
		if (distance < closest) {
			ret = pt;
			closest = distance;
		}
	}
	return ret;
}

auto Map::get_random_home_point() const -> sf::Vector2f { return random::random_element(home_points); }

auto Map::is_toxic() const -> bool { return (get_style_id() == 7 && !is_interior()) || has_property(MapProperties::toxic); }

auto Map::get_ambience_balance() const -> float { return ambience_balance.get(); }

dj::Json const& Map::get_json_data(automa::ServiceProvider& svc) const {
	auto rid = room_id;
	auto const it = std::find_if(svc.data.map_jsons.begin(), svc.data.map_jsons.end(), [rid](auto const& r) { return r.id == rid; });
	return it->metadata;
}

std::size_t Map::get_index_at_position(sf::Vector2f position) { return get_middleground()->grid.get_index_at_position(position); }

int Map::get_tile_value_at_position(sf::Vector2f position) { return get_middleground()->grid.get_cell(get_index_at_position(position)).value; }

Tile& Map::get_cell_at_position(sf::Vector2f position) { return get_middleground()->grid.cells.at(get_index_at_position(position)); }

enemy::Enemy* Map::get_enemy(int id) {
	auto it = std::find_if(enemy_catalog.enemies.begin(), enemy_catalog.enemies.end(), [id](auto const& e) { return e->get_id() == id; });
	if (it == enemy_catalog.enemies.end()) { return nullptr; }
	return it->get();
}

MapAttributes::MapAttributes(dj::Json const& in) {
	// map properties
	if (in["properties"]["environmental_randomness"].as_bool()) { properties.set(MapProperties::environmental_randomness); }
	if (in["properties"]["day_night_shift"].as_bool()) { properties.set(MapProperties::day_night_shift); }
	if (in["properties"]["timer"].as_bool()) { properties.set(MapProperties::timer); }
	if (in["properties"]["lighting"].as_bool()) { properties.set(MapProperties::lighting); }
	if (in["properties"]["interior"].as_bool()) { properties.set(MapProperties::interior); }
	if (in["properties"]["toxic"].as_bool()) { properties.set(MapProperties::toxic); }
	if (in["properties"]["sky_limit"]) { sky_limit = in["properties"]["sky_limit"].as<int>(); }
	if (in["minimap"].as_bool()) { properties.set(MapProperties::minimap); }

	if (in["camera_effects"]) {
		shake_properties.frequency = in["camera_effects"]["shake"]["frequency"].as<int>();
		shake_properties.energy = in["camera_effects"]["shake"]["energy"].as<float>();
		shake_properties.start_time = in["camera_effects"]["shake"]["start_time"].as<float>();
		shake_properties.dampen_factor = in["camera_effects"]["shake"]["dampen_factor"].as<int>();
		shake_cooldown = util::Cooldown{in["camera_effects"]["shake"]["frequency_in_seconds"].as<int>()};
		shake_properties.shaking = shake_properties.frequency > 0;
		shake_properties.chance = in["camera_effects"]["shake"]["chance"].as<float>();
		shake_cooldown.start();
	}

	music = in["music"].as_string();
	ambience = in["ambience"].as_string();
	for (auto& entry : in["atmosphere"].as_array()) { atmosphere.add(entry.as_string()); }

	special_drop_id = in["special_drop_id"].as<int>();
}

void MapAttributes::serialize(dj::Json& out) {
	out["properties"]["environmental_randomness"] = properties.test(fornani::world::MapProperties::environmental_randomness);
	out["properties"]["day_night_shift"] = properties.test(fornani::world::MapProperties::day_night_shift);
	out["properties"]["timer"] = properties.test(fornani::world::MapProperties::timer);
	out["properties"]["lighting"] = properties.test(fornani::world::MapProperties::lighting);
	out["properties"]["interior"] = properties.test(fornani::world::MapProperties::interior);
	out["properties"]["toxic"] = properties.test(fornani::world::MapProperties::toxic);
	out["properties"]["sky_limit"] = sky_limit;

	out["music"] = music;
	NANI_LOG_DEBUG(m_logger, "Serialized music: {}", music);
	out["ambience"] = ambience;
	for (auto& atmo : atmosphere) { out["atmosphere"].push_back(atmo); }

	out["special_drop_id"] = special_drop_id;
}

void Map::update_balance(automa::ServiceProvider& svc) {
	music_balance.update(svc.ticker.dt.count());
	ambience_balance.update(svc.ticker.dt.count());
	if (m_attributes.properties.test(MapProperties::interior)) {
		set_target_balance(0.f, audio::BalanceTarget::ambience);
	} else {
		set_target_balance(cooldowns.fade_obscured.get_normalized(), audio::BalanceTarget::ambience);
	}
}

} // namespace fornani::world
