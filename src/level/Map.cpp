
#include "Map.hpp"
#include <imgui.h>
#include "../entities/player/Player.hpp"
#include "../gui/InventoryWindow.hpp"
#include "../gui/Portrait.hpp"
#include "../service/ServiceProvider.hpp"
#include "../setup/EnumLookups.hpp"

#include "../utils/Math.hpp"

namespace world {

Map::Map(automa::ServiceProvider& svc, player::Player& player, gui::Console& console) : player(&player), enemy_catalog(svc), save_point(svc), transition(svc, 256), m_services(&svc), m_console(&console) {}

void Map::load(automa::ServiceProvider& svc, std::string_view room) {

	std::string room_str = svc.data.finder.resource_path + room.data();
	metadata = dj::Json::from_file((room_str + "/meta.json").c_str());
	assert(!metadata.is_null());
	tiles = dj::Json::from_file((room_str + "/tile.json").c_str());
	assert(!tiles.is_null());
	inspectable_data = dj::Json::from_file((room_str + "/inspectables.json").c_str());

	// get npc data
	if (!metadata.is_null()) {
		auto const& meta = metadata["meta"];
		room_id = meta["room_id"].as<int>();
		dimensions.x = meta["dimensions"][0].as<int>();
		dimensions.y = meta["dimensions"][1].as<int>();
		chunk_dimensions.x = meta["chunk_dimensions"][0].as<int>();
		chunk_dimensions.y = meta["chunk_dimensions"][1].as<int>();
		real_dimensions = {(float)dimensions.x * svc.constants.cell_size, (float)dimensions.y * svc.constants.cell_size};
		for (int i = 0; i < NUM_LAYERS; ++i) { layers.push_back(Layer(i, (i == MIDDLEGROUND), dimensions)); }

		if (meta["music"].is_string()) {
			svc.music.load(meta["music"].as_string());
			svc.music.play_looped(20);
		}

		auto style_value = meta["style"].as<int>();
		style_label = svc.data.map_styles["styles"][style_value]["label"].as_string();
		style_id = svc.data.map_styles["styles"][style_value]["id"].as<int>();
		if (svc.greyblock_mode()) { style_id = 20; }
		native_style_id = svc.data.map_styles["styles"][style_value]["id"].as<int>();
		background = std::make_unique<bg::Background>(svc, meta["background"].as<int>());
		styles.breakables = meta["styles"]["breakables"].as<int>();

		for (auto& entry : metadata["npcs"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>();
			pos.y = entry["position"][1].as<float>();
			npcs.push_back(npc::NPC(svc, entry["id"].as<int>()));
			for (auto& convo : entry["suites"].array_view()) { npcs.back().push_conversation(convo.as_string()); }
			npcs.back().set_position_from_scaled(pos);
		}

		for (auto& entry : metadata["portals"].array_view()) {
			sf::Vector2<uint32_t> pos{};
			sf::Vector2<uint32_t> dim{};
			pos.x = entry["position"][0].as<int>();
			pos.y = entry["position"][1].as<int>();
			dim.x = entry["dimensions"][0].as<int>();
			dim.y = entry["dimensions"][1].as<int>();
			portals.push_back(entity::Portal(dim, pos));
			portals.back().source_map_id = entry["source_id"].as<int>();
			portals.back().destination_map_id = entry["destination_id"].as<int>();
			portals.back().activate_on_contact = (bool)entry["activate_on_contact"].as_bool();
			portals.back().update();
		}

		auto const& savept = metadata["save_point"];
		auto save_id = svc.state_controller.save_point_id;
		save_point.id = savept.contains("position") ? room_id : -1;
		std::cout << "Save Point ID loaded to map: " << save_point.id << "\n";
		std::cout << "Room ID (should match above): " << room_id << "\n";
		save_point.scaled_position.x = savept["position"][0].as<int>();
		save_point.scaled_position.y = savept["position"][1].as<int>();

		for (auto& entry : metadata["chests"].array_view()) {
			sf::Vector2<float> pos{};
			pos.x = entry["position"][0].as<float>();
			pos.y = entry["position"][1].as<float>();
			chests.push_back(entity::Chest(svc));
			chests.back().set_id(entry["id"].as<int>());
			chests.back().set_item(entry["item_id"].as<int>());
			chests.back().set_amount(entry["amount"].as<int>());
			chests.back().set_rarity(entry["rarity"].as<float>());
			if (entry["type"].as<int>() == 1) { chests.back().set_type(entity::ChestType::gun); }
			if (entry["type"].as<int>() == 2) { chests.back().set_type(entity::ChestType::orbs); }
			if (entry["type"].as<int>() == 3) { chests.back().set_type(entity::ChestType::item); }
			chests.back().set_position_from_scaled(pos);
		}

		for (auto& entry : metadata["animators"].array_view()) {
			sf::Vector2<int> scaled_dim{};
			sf::Vector2<int> scaled_pos{};
			scaled_pos.x = entry["position"][0].as<int>();
			scaled_pos.y = entry["position"][1].as<int>();
			scaled_dim.x = entry["dimensions"][0].as<int>();
			scaled_dim.y = entry["dimensions"][1].as<int>();
			auto lg = scaled_dim.x == 2;
			auto a = entity::Animator(svc, scaled_pos, lg);
			a.id = entry["id"].as<int>();
			a.automatic = (bool)entry["automatic"].as_bool();
			a.foreground = (bool)entry["foreground"].as_bool();
			animators.push_back(a);
		}

		for (auto& entry : metadata["inspectables"].array_view()) {
			sf::Vector2<uint32_t> dim{};
			sf::Vector2<uint32_t> pos{};
			auto key = entry["key"].as_string();
			pos.x = entry["position"][0].as<int>();
			pos.y = entry["position"][1].as<int>();
			dim.x = entry["dimensions"][0].as<int>();
			dim.y = entry["dimensions"][1].as<int>();
			inspectables.push_back(entity::Inspectable(dim, pos, key));
			inspectables.back().activate_on_contact = (bool)entry["activate_on_contact"].as_bool();
		}

		for (auto& entry : metadata["enemies"].array_view()) {
			int id{};
			sf::Vector2<int> pos{};
			pos.x = entry["position"][0].as<int>();
			pos.y = entry["position"][1].as<int>();
			enemy_catalog.push_enemy(svc, *this, *m_console, entry["id"].as<int>());
			enemy_catalog.enemies.back()->set_position({(float)(pos.x * svc.constants.cell_size), (float)(pos.y * svc.constants.cell_size)});
			enemy_catalog.enemies.back()->get_collider().physics.zero();
		}

		for (auto& entry : metadata["platforms"].array_view()) {
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
	}

	// tiles
	int layer_counter{};
	for (auto& layer : layers) {
		int cell_counter{};
		layer.grid = Grid(dimensions);
		for (auto& cell : tiles["layers"][layer_counter].array_view()) {
			layer.grid.cells.at(cell_counter).value = cell.as<int>();
			++cell_counter;
		}
		layer.grid.seed_vertices();
		++layer_counter;
	}

	generate_collidable_layer();
	generate_layer_textures(svc);

	player->map_reset();

	transition.fade_in = true;
	minimap = sf::View(sf::FloatRect(0.0f, 0.0f, svc.constants.screen_dimensions.x * 2.f, svc.constants.screen_dimensions.y * 2.f));
	minimap.setViewport(sf::FloatRect(0.75f, 0.75f, 0.2f, 0.2f));
	loading.start(2);
}

void Map::update(automa::ServiceProvider& svc, gui::Console& console, gui::InventoryWindow& inventory_window) {
	loading.update();
	if (loading.running()) { generate_layer_textures(svc); } // band-aid fix for weird artifacting for 1x1 levels
	flags.state.reset(LevelState::camera_shake);

	if(flags.state.test(LevelState::spawn_enemy)) {
		for (auto& spawn : enemy_spawns) {
			enemy_catalog.push_enemy(*m_services, *this, *m_console, spawn.id);
			enemy_catalog.enemies.back()->set_position(spawn.pos);
			enemy_catalog.enemies.back()->get_collider().physics.zero();
			effects.push_back(entity::Effect(*m_services, spawn.pos, {}, 0, 4));
		}
		enemy_spawns.clear();
		flags.state.reset(LevelState::spawn_enemy);
	}

	console.update(svc);
	inventory_window.update(svc, *player);

	player->collider.reset();
	for (auto& a : player->antennae) { a.collider.reset(); }
	player->ledge_height = player->collider.detect_ledge_height(*this);
	if (off_the_bottom(player->collider.physics.position)) { player->kill(); }

	for (auto& grenade : active_grenades) {
		if (player->shielding() && player->controller.get_shield().sensor.within_bounds(grenade.bounding_box)) {
			player->controller.get_shield().damage();
			grenade.physics.velocity *= -1.f;
		}
		if (grenade.detonated() && grenade.sensor.within_bounds(player->collider.hurtbox)) { player->hurt(grenade.get_damage()); }
		for (auto& enemy : enemy_catalog.enemies) {
			if (grenade.detonated() && grenade.sensor.within_bounds(enemy->get_collider().hurtbox)) {
				enemy->hurt();
				enemy->health.inflict(grenade.get_damage());
				enemy->health_indicator.add(grenade.get_damage());
				if (enemy->just_died() && enemy->spawn_loot()) {
					active_loot.push_back(item::Loot(svc, enemy->get_attributes().drop_range, enemy->get_attributes().loot_multiplier, enemy->get_collider().bounding_box.position));
					svc.soundboard.flags.frdog.set(audio::Frdog::death);
				}
			}
		}
	}

	player->collider.detect_map_collision(*this);

	// i need to refactor this...
	for (auto& index : collidable_indeces) {
		auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
		// damage player if spikes
		if (cell.is_spike() && player->collider.hurtbox.overlaps(cell.bounding_box)) { player->hurt(1); }
		if (cell.is_death_spike() && player->collider.hurtbox.overlaps(cell.bounding_box)) { player->hurt(64); }

		for (auto& grenade : active_grenades) {
			for (auto& breakable : breakables) {
				if (grenade.detonated() && grenade.sensor.within_bounds(breakable.get_bounding_box())) { breakable.destroy(); }
			}
		}
		for (auto& proj : active_projectiles) {

			// should be, simply:
			// cell.update(svc, player, proj, *this);
			// or something similar

			if (!nearby(cell.bounding_box, proj.bounding_box)) {
				continue;
			} else {
				cell.collision_check = true;
				if ((proj.bounding_box.overlaps(cell.bounding_box) && cell.is_occupied())) {
					if (!cell.is_collidable()) { continue; }
					if (!proj.stats.transcendent) {
						if (!proj.destruction_initiated()) {
							effects.push_back(entity::Effect(svc, proj.destruction_point + proj.physics.position, {}, proj.effect_type(), 2));
							if (proj.direction.lr == dir::LR::neutral) { effects.back().rotate(); }
						}
						proj.destroy(false);
					}
					if (proj.stats.spring && cell.is_hookable()) {
						if (proj.hook.grapple_flags.test(arms::GrappleState::probing)) {
							proj.hook.spring.set_anchor(cell.middle_point());
							proj.hook.grapple_triggers.set(arms::GrappleTriggers::found);
						}
						handle_grappling_hook(svc, proj);
					}
				}
			}
		}
	}

	manage_projectiles(svc);

	for (auto& proj : active_projectiles) {
		if (proj.state.test(arms::ProjectileState::destruction_initiated)) { continue; }
		for (auto& platform : platforms) { platform.on_hit(svc, *this, proj); }
		for (auto& breakable : breakables) { breakable.on_hit(svc, *this, proj); }
		for (auto& enemy : enemy_catalog.enemies) { enemy->on_hit(svc, *this, proj); }

		if (player->shielding() && player->controller.get_shield().sensor.within_bounds(proj.bounding_box)) { player->controller.get_shield().damage(proj.stats.base_damage * player->player_stats.shield_dampen); }
		if (proj.bounding_box.overlaps(player->collider.hurtbox) && proj.team != arms::TEAMS::NANI) {
			player->hurt(proj.stats.base_damage);
			proj.destroy(false);
		}
	}

	for (auto& enemy : enemy_catalog.enemies) {
		enemy->unique_update(svc, *this, *player);
		enemy->handle_player_collision(*player);
	}
	enemy_catalog.update();

	for (auto& loot : active_loot) { loot.update(svc, *this, *player); }
	for (auto& grenade : active_grenades) { grenade.update(svc, *player, *this); }
	for (auto& emitter : active_emitters) { emitter.update(svc, *this); }
	for (auto& chest : chests) { chest.update(svc, *this, console, *player); }
	for (auto& npc : npcs) { npc.update(svc, *this, console, *player); }
	for (auto& portal : portals) { portal.handle_activation(svc, *player, room_id, transition.fade_out, transition.done); }
	for (auto& inspectable : inspectables) { inspectable.update(svc, *player, console, inspectable_data); }
	for (auto& animator : animators) { animator.update(*player); }
	for (auto& effect : effects) { effect.update(svc, *this); }
	for (auto& platform : platforms) { platform.update(svc, *player); }
	for (auto& breakable : breakables) {
		breakable.update(svc);
		breakable.handle_collision(player->collider);
	}
	if (save_point.id != -1) { save_point.update(svc, *player, console); }

	std::erase_if(effects, [](auto const& e) { return e.done(); });

	player->collider.reset_ground_flags();
	// check if player died
	if (!player->is_dead() && !game_over) {
		svc.soundboard.flags.player.set(audio::Player::death);
		game_over = true;
	}

	if (game_over) {
		transition.fade_out = true;
		if (transition.done) {
			player->start_over();
			svc.state_controller.actions.set(automa::Actions::player_death);
			svc.state_controller.actions.set(automa::Actions::trigger);
		}
	}

	if (svc.ticker.every_x_frames(1)) { transition.update(); }

	console.clean_off_trigger();
	inventory_window.clean_off_trigger();
	inventory_window.info.clean_off_trigger();
}

void Map::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	// check for a switch to greyblock mode
	if (svc.debug_flags.test(automa::DebugFlags::greyblock_trigger)) {
		style_id = style_id == 20 ? native_style_id : 20;
		generate_layer_textures(svc);
		svc.debug_flags.reset(automa::DebugFlags::greyblock_trigger);
	}

	for (auto& chest : chests) { chest.render(svc, win, cam); }
	for (auto& npc : npcs) { npc.render(svc, win, cam); }
	for (auto& emitter : active_emitters) { emitter.render(svc, win, cam); }
	for (auto& grenade : active_grenades) { grenade.render(svc, win, cam); }
	player->render(svc, win, cam);
	for (auto& enemy : enemy_catalog.enemies) { enemy->render(svc, win, cam); }
	for (auto& enemy : enemy_catalog.enemies) { enemy->unique_render(svc, win, cam); }
	for (auto& proj : active_projectiles) { proj.render(svc, *player, win, cam); }
	for (auto& loot : active_loot) { loot.render(svc, win, cam); }
	for (auto& platform : platforms) { platform.render(svc, win, cam); }
	for (auto& breakable : breakables) { breakable.render(svc, win, cam); }

	for (auto& animator : animators) {
		if (!animator.foreground) { animator.render(svc, win, cam); }
	}

	if (save_point.id != -1) { save_point.render(svc, win, cam); }

	// map foreground tiles
	for (int i = 4; i < NUM_LAYERS; ++i) {
		if (svc.greyblock_mode() && i != 4) { continue; }
		layer_textures.at(i).display();
		layer_sprite.setTexture(layer_textures.at(i).getTexture());
		layer_sprite.setPosition(-cam);
		win.draw(layer_sprite);
	}

	for (auto& effect : effects) { effect.render(svc, win, cam); }

	player->render_indicators(svc, win, cam);
	for (auto& enemy : enemy_catalog.enemies) { enemy->render_indicators(svc, win, cam); }
	for (auto& enemy : enemy_catalog.enemies) { enemy->gui_render(svc, win, cam); }

	if (svc.greyblock_mode()) {
		for (auto& index : collidable_indeces) {
			auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
			cell.drawbox.setPosition(cell.position - cam);
			win.draw(cell.drawbox);
		}
	}

	if (real_dimensions.y < svc.constants.screen_dimensions.y) {
		float ydiff = (svc.constants.screen_dimensions.y - real_dimensions.y) / 2;
		borderbox.setFillColor(svc.styles.colors.ui_black);
		borderbox.setSize({(float)svc.constants.screen_dimensions.x, ydiff});
		borderbox.setPosition(0.0f, 0.0f);
		win.draw(borderbox);

		borderbox.setPosition(0.0f, real_dimensions.y + ydiff);
		win.draw(borderbox);
	}
	if (real_dimensions.x < svc.constants.screen_dimensions.x) {
		float xdiff = (svc.constants.screen_dimensions.x - real_dimensions.x) / 2;
		borderbox.setFillColor(svc.styles.colors.ui_black);
		borderbox.setSize({xdiff, (float)svc.constants.screen_dimensions.y});
		borderbox.setPosition(0.0f, 0.0f);
		win.draw(borderbox);

		borderbox.setPosition(real_dimensions.x + xdiff, 0.0f);
		win.draw(borderbox);
	}

	for (auto& portal : portals) { portal.render(win, cam); }

	for (auto& animator : animators) {
		if (animator.foreground) { animator.render(svc, win, cam); }
	}

	if (svc.greyblock_mode()) {
		for (auto& inspectable : inspectables) { inspectable.render(win, cam); }
	}

	// render minimap
	if (show_minimap) {
		win.setView(minimap);
		for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
			minimap_tile.setPosition(cell.position.x - cam.x, cell.position.y - cam.y);
			minimap_tile.setSize(sf::Vector2<float>{(float)cell.bounding_box.dimensions.x, (float)cell.bounding_box.dimensions.y});
			if (cell.value > 0) {
				auto color = svc.styles.colors.ui_white;
				color.a = 120;
				minimap_tile.setFillColor(color);
				win.draw(minimap_tile);

			} else {
				minimap_tile.setFillColor(sf::Color{20, 20, 20, 120});
				win.draw(minimap_tile);
			}
		}
		minimap_tile.setPosition(player->collider.physics.position.x - cam.x, player->collider.physics.position.y - cam.y);
		minimap_tile.setFillColor(sf::Color{240, 240, 240, 180});
		win.draw(minimap_tile);

		win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y}));
	}
}

void Map::render_background(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (!svc.greyblock_mode()) {
		background->render(svc, win, cam, real_dimensions);
		for (int i = 0; i < 4; ++i) {
			layer_textures.at(i).display();
			layer_sprite.setTexture(layer_textures.at(i).getTexture());
			layer_sprite.setPosition(-cam);
			win.draw(layer_sprite);
		}
	} else {
		sf::RectangleShape box{};
		box.setPosition(0, 0);
		box.setFillColor(flcolor::black);
		box.setSize({(float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y});
		win.draw(box);
	}
}

void Map::render_console(automa::ServiceProvider& svc, gui::Console& console, sf::RenderWindow& win) {
	if (console.flags.test(gui::ConsoleFlags::active)) { console.render(win); }
	console.write(win, false);
}

void Map::spawn_projectile_at(automa::ServiceProvider& svc, arms::Weapon& weapon, sf::Vector2<float> pos) {
	if (weapon.attributes.grenade) { active_grenades.push_back(arms::Grenade(svc, pos, weapon.firing_direction)); }
	active_projectiles.push_back(weapon.projectile);
	active_projectiles.back().set_sprite(svc);
	active_projectiles.back().set_position(pos);
	active_projectiles.back().seed(svc);
	active_projectiles.back().update(svc, *player);
	active_projectiles.back().sync_position();
	if (active_projectiles.back().stats.boomerang) { active_projectiles.back().set_boomerang_speed(); }
	if (active_projectiles.back().stats.spring) {
		active_projectiles.back().set_hook_speed();
		active_projectiles.back().hook.grapple_flags.set(arms::GrappleState::probing);
	}

	active_emitters.push_back(vfx::Emitter(svc, weapon.barrel_point, weapon.emitter_dimensions, weapon.emitter_type, weapon.emitter_color, weapon.firing_direction));
}

void Map::spawn_enemy(int id, sf::Vector2<float> pos) {
	enemy_spawns.push_back({pos, id});
	flags.state.set(LevelState::spawn_enemy);
}

void Map::manage_projectiles(automa::ServiceProvider& svc) {
	for (auto& proj : active_projectiles) {
		proj.update(svc, *player);
		if (proj.whiffed() && !proj.poofed() && !proj.made_contact()) {
			effects.push_back(entity::Effect(svc, proj.physics.position, proj.physics.velocity * 0.1f, proj.effect_type(), 8));
			proj.state.set(arms::ProjectileState::poof);
		}
	}
	for (auto& emitter : active_emitters) { emitter.update(svc, *this); }

	std::erase_if(active_projectiles, [](auto const& p) { return p.state.test(arms::ProjectileState::destroyed); });
	std::erase_if(active_grenades, [](auto const& g) { return g.detonated(); });
	std::erase_if(active_emitters, [](auto const& p) { return p.done(); });

	if (!player->arsenal.loadout.empty()) {
		if (player->fire_weapon()) {
			spawn_projectile_at(svc, player->equipped_weapon(), player->equipped_weapon().barrel_point);
			++player->equipped_weapon().active_projectiles;
			player->equipped_weapon().shoot();
			if (!player->equipped_weapon().attributes.automatic) { player->controller.set_shot(false); }
		}
	}
}

void Map::generate_collidable_layer() {
	layers.at(MIDDLEGROUND).grid.check_neighbors();
	for (auto& cell : layers.at(MIDDLEGROUND).grid.cells) {
		if ((!cell.surrounded && cell.is_occupied() && !cell.is_breakable())) { collidable_indeces.push_back(cell.one_d_index); }
		if (cell.is_breakable()) { breakables.push_back(Breakable(*m_services, cell.position, styles.breakables)); }
	}
}

void Map::generate_layer_textures(automa::ServiceProvider& svc) {
	for (auto& layer : layers) {
		layer_textures.at((int)layer.render_order).clear(sf::Color::Transparent);
		layer_textures.at((int)layer.render_order).create(layer.grid.dimensions.x * svc.constants.cell_size, layer.grid.dimensions.y * svc.constants.cell_size);
		for (auto& cell : layer.grid.cells) {
			if (cell.is_occupied() && !cell.is_breakable()) {
				int x_coord = (cell.value % svc.constants.tileset_scaled.x) * svc.constants.cell_size;
				int y_coord = std::floor(cell.value / svc.constants.tileset_scaled.x) * svc.constants.cell_size;
				tile_sprite.setTexture(svc.assets.tilesets.at(style_id));
				tile_sprite.setTextureRect(sf::IntRect({x_coord, y_coord}, {(int)svc.constants.cell_size, (int)svc.constants.cell_size}));
				tile_sprite.setPosition(cell.position);
				layer_textures.at((int)layer.render_order).draw(tile_sprite);
			}
		}
	}
}

bool Map::check_cell_collision(shape::Collider collider) {
	for (auto& index : collidable_indeces) {
		auto& cell = layers.at(MIDDLEGROUND).grid.cells.at(index);
		if (!nearby(cell.bounding_box, collider.bounding_box)) {
			continue;
		} else {
			// check vicinity so we can escape early
			if (!collider.vicinity.overlaps(cell.bounding_box)) {
				continue;
			} else if (!cell.is_solid()) {
				continue;
			} else {
				if (cell.value > 0 && collider.predictive_combined.SAT(cell.bounding_box)) { return true; }
			}
		}
	}
	return false;
}

void Map::handle_grappling_hook(automa::ServiceProvider& svc, arms::Projectile& proj) {
	// do this first block once
	if (proj.hook.grapple_triggers.test(arms::GrappleTriggers::found) && !proj.hook.grapple_flags.test(arms::GrappleState::anchored) && !proj.hook.grapple_flags.test(arms::GrappleState::snaking)) {
		proj.hook.spring.set_bob(player->apparent_position);
		proj.hook.grapple_triggers.reset(arms::GrappleTriggers::found);
		proj.hook.grapple_flags.set(arms::GrappleState::anchored);
		proj.hook.grapple_flags.reset(arms::GrappleState::probing);
		proj.hook.spring.set_force(proj.stats.spring_constant);
		proj.hook.spring.variables.physics.acceleration += player->collider.physics.acceleration;
		proj.hook.spring.variables.physics.velocity += player->collider.physics.velocity;
	}
	if (player->controller.hook_held() && proj.hook.grapple_flags.test(arms::GrappleState::anchored)) {
		proj.hook.spring.variables.physics.acceleration += player->collider.physics.acceleration;
		proj.hook.spring.variables.physics.acceleration.x += player->controller.horizontal_movement();
		proj.lock_to_anchor();
		proj.hook.spring.update(svc);

		// update rest length
		auto next_length = proj.stats.spring_slack * abs(player->collider.physics.position.y - proj.hook.spring.get_anchor().y);
		next_length = std::clamp(next_length, lookup::min_hook_length, lookup::max_hook_length);
		proj.hook.spring.set_rest_length(next_length);

		// break out if player is far away from bob. we don't want the player to teleport.
		auto distance = util::magnitude(player->collider.physics.position - proj.hook.spring.get_bob());
		if (distance > 32.f) { proj.hook.break_free(*player); }

		// handle map collisions while anchored
		player->collider.predictive_combined.set_position(proj.hook.spring.variables.physics.position);
		if (check_cell_collision(player->collider)) {
			player->collider.physics.zero();
		} else {
			player->collider.physics.position = proj.hook.spring.variables.physics.position - player->collider.dimensions / 2.f;
		}
		player->collider.sync_components();
	} else if (proj.hook.grapple_flags.test(arms::GrappleState::anchored)) {
		proj.hook.break_free(*player);
	}

	if (player->controller.released_hook() && !proj.hook.grapple_flags.test(arms::GrappleState::snaking)) { proj.hook.break_free(*player); }
}

void Map::shake_camera() { flags.state.set(LevelState::camera_shake); }

sf::Vector2<float> Map::get_spawn_position(int portal_source_map_id) {
	for (auto& portal : portals) {
		if (portal.source_map_id == portal_source_map_id) { return (portal.position); }
	}
	return Vec(300.f, 390.f);
}

bool Map::nearby(shape::Shape& first, shape::Shape& second) const {
	return abs(first.position.x + first.dimensions.x * 0.5f - second.position.x) < lookup::unit_size_f * collision_barrier && abs(first.position.y - second.position.y) < lookup::unit_size_f * collision_barrier;
}

} // namespace world
