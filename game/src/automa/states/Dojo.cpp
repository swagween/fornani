
#include "fornani/automa/states/Dojo.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name)
	: GameState(svc, player, scene, room_number), map(svc, player, console), gui_map(svc, player, console){
	svc.menu_controller.reset_vendor_dialog();
	open_vendor = false;
	if (!svc.data.room_discovered(room_number)) {
		svc.data.discovered_rooms.push_back(room_number);
		svc.stats.world.rooms_discovered.update();
	}
	player.reset_flags();
	// the following should only happen for the editor demo
	if (!svc.data.exists(room_number)) {
		svc.data.rooms.push_back(room_number);
		svc.data.load_data(room_name.data());
	} else {
		map.load(svc, room_number);
		bake_maps(svc, {map.room_id}, true);
		auto m_id = map.room_id;
		bake_maps(svc, svc.data.rooms);
	}
	hud.orient(svc, player); // reset hud position to corner
	svc.soundboard.turn_on();

	// TODO: refactor player initialization
	player.collider.physics.zero();

	bool found_one{};
	// only search for door entry if room was not loaded from main menu and player didn't die
	if (!svc.state_controller.actions.test(Actions::save_loaded) && !svc.state_controller.actions.test(Actions::player_death)) {
		for (auto& portal : map.portals) {
			if (portal.get_destination() == svc.state_controller.source_id) {
				found_one = true;
				sf::Vector2<float> spawn_position{portal.position.x + (portal.dimensions.x * 0.5f), portal.position.y + portal.dimensions.y - player.height()};
				player.set_position(spawn_position, true);
				camera.force_center(player.anchor_point);
				if (portal.activate_on_contact() && portal.is_left_or_right()) {
					enter_room.start(90);
				} else {
					if (!portal.already_open()) { portal.close(); }
					player.set_idle();
				}
				if (portal.is_bottom()) {
					player.collider.physics.acceleration.y = -player.physics_stats.jump_velocity;
					player.collider.physics.acceleration.x = player.controller.facing_left() ? -player.physics_stats.x_acc : player.physics_stats.x_acc;
				}
			}
		}
	}
	if (!found_one) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		player.set_position(player_pos);
	}

	if (player.piggybacker) { player.piggybacker.value().set_position(player.collider.physics.position); }

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player.is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player.visit_history.push_room(room_number);

	player.controller.prevent_movement();
	inventory_window.update_wardrobe(svc, player);
	console.nani_portrait.set_custom_portrait(inventory_window.get_wardrobe_sprite());
	loading.start();
}

void Dojo::tick_update(ServiceProvider& svc) {
	svc.a11y.set_action_ctx_bar_enabled(false);

	loading.is_complete() && !vendor_dialog ? svc.app_flags.set(AppFlags::in_game) : svc.app_flags.reset(AppFlags::in_game);
	loading.update();

	svc.soundboard.play_sounds(svc, map.get_echo_count(), map.get_echo_rate());

	if (pause_window.active()) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { toggle_pause_menu(svc); }
		pause_window.update(svc, console, false);
		return;
	}

	svc.world_clock.update(svc);

	if (vendor_dialog) {
		if (open_vendor) {
			map.transition.end();
			open_vendor = false;
		}
		map.transition.update(*player);
		vendor_dialog.value().update(svc, map, *player);
		if (!vendor_dialog.value().is_open()) {
			if (vendor_dialog.value().made_profit()) { svc.soundboard.flags.item.set(audio::Item::orb_max); }
			vendor_dialog = {};
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			inventory_window.update_wardrobe(svc, *player);
		}
		return;
	}
	if (console.is_complete()) {
		if (inventory_window.active()) {
			if (inventory_window.is_inventory()) {
				svc.controller_map.set_action_set(config::ActionSet::Inventory);
			} else if (inventory_window.is_minimap()) {
				svc.controller_map.set_action_set(config::ActionSet::Map);
			}
		} else {
			svc.controller_map.set_action_set(config::ActionSet::Platformer);
		}
	} else {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
	}

	// pause the game if controller was disconnected
	if (svc.controller_map.process_gamepad_disconnection()) { toggle_pause_menu(svc); }

	if (!svc.no_menu()) {
		if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_inventory).triggered || svc.controller_map.digital_action_status(config::DigitalAction::inventory_close).triggered ||
			svc.controller_map.digital_action_status(config::DigitalAction::map_open_inventory).triggered) {

			if (inventory_window.active() && !inventory_window.is_inventory()) {
				inventory_window.switch_modes(svc);
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			} else {
				toggle_inventory(svc);
				if (!inventory_window.is_inventory()) { inventory_window.switch_modes(svc); }
			}
		}
		if ((svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_map).triggered || svc.controller_map.digital_action_status(config::DigitalAction::map_close).triggered ||
			 svc.controller_map.digital_action_status(config::DigitalAction::inventory_open_map).triggered) &&
			player->has_map()) {
			if (inventory_window.active() && !inventory_window.is_minimap()) {
				inventory_window.switch_modes(svc);
				svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			} else {
				toggle_inventory(svc);
				if (!inventory_window.is_minimap()) { inventory_window.switch_modes(svc); }
			}
		}
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { toggle_pause_menu(svc); }

	if (console.is_complete()) {
		if (svc.menu_controller.vendor_dialog_opened()) {
			map.transition.start();
			open_vendor = true;
		}
		if (open_vendor && map.transition.is_done()) {
			vendor_dialog = gui::VendorDialog(svc, map, *player, svc.menu_controller.get_menu_id());
			svc.controller_map.set_action_set(config::ActionSet::Menu);
			svc.soundboard.flags.console.set(audio::Console::menu_open);
		}
	}

	if (player->visit_history.traveled_far() || svc.data.marketplace.at(3).inventory.items.empty()) {
		svc.random.set_vendor_seed();
		for (auto& vendor : svc.data.marketplace) { vendor.second.generate_inventory(svc); }
		player->visit_history.clear();
	}

	enter_room.update();
	if (console.is_complete() && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }
	if (enter_room.running()) { player->controller.autonomous_walk(); }

	// A.update(svc);
	// B.update(svc);
	// auto mtv = A.bounding_box.testCollisionGetMTV(B.bounding_box, A.bounding_box);
	// if (svc.ticker.every_x_ticks(400)) { std::cout << "MYT x: " << mtv.x << "\n"; }
	/*circle.update(svc);
	circle.sensor.deactivate();
	for (auto& cell : map.get_middleground().grid.cells) {
		if (circle.collides_with(cell.bounding_box) && cell.is_collidable()) {
			circle.sensor.activate();
			auto mtv = circle.sensor.get_MTV(cell.bounding_box);
			if (svc.ticker.every_x_ticks(50)) { std::cout << "MTV: x[ " << mtv.x << " ] : y[ " << mtv.y << " ]\n"; }
		}
	}*/

	player->update(map, console, inventory_window);
	map.update(svc, console, inventory_window);

	if (map.camera_shake()) { camera.begin_shake(); }
	camera.center(player->anchor_point);
	camera.update(svc);
	camera.restrict_movement(map.real_dimensions);

	map.debug_mode = debug_mode;

	player->controller.clean();
	player->flags.triggers = {};

	pause_window.update(svc, console, true);
	map.background->update(svc);
	hud.update(svc, *player);

	console.end_tick();
}

void Dojo::frame_update(ServiceProvider& svc) {
	pause_window.render_update(svc);
	pause_window.clean_off_trigger();
}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {

	// B.physics.position = sf::Vector2<float>(sf::Mouse::getPosition());
	// circle.set_position(sf::Vector2<float>(sf::Mouse::getPosition()) + camera.get_position());

	map.render_background(svc, win, camera.get_position());
	map.render(svc, win, camera.get_position());

	if (!svc.greyblock_mode() && !svc.hide_hud()) { hud.render(*player, win); }
	inventory_window.render(svc, *player, win, camera.get_position());
	pause_window.render(svc, *player, win);
	if (vendor_dialog) { vendor_dialog.value().render(svc, win, *player, map); }
	map.soft_reset.render(win);
	map.transition.render(win);
	map.render_console(svc, console, win);
	player->tutorial.render(win);
	if (svc.debug_mode()) { map.debug(); }

	// A.render(win, {});
	// B.render(win, {});
	// circle.render(win, camera.get_position());
}

void Dojo::toggle_inventory(ServiceProvider& svc) {
	if (pause_window.active()) { return; }
	// refresh potential new console portrait
	console.nani_portrait.set_custom_portrait(inventory_window.get_wardrobe_sprite());
	if (inventory_window.active()) {
		svc.soundboard.flags.console.set(audio::Console::done);
		inventory_window.close();
	} else {
		inventory_window.minimap.update(svc, map, *player);
		inventory_window.open(svc, *player);
		svc.soundboard.flags.console.set(audio::Console::menu_open);
		inventory_window.set_item_size(static_cast<int>(player->catalog.categories.inventory.items.size()));
	}
}

void Dojo::toggle_pause_menu(ServiceProvider& svc) {
	if (pause_window.active()) {
		pause_window.close();
		svc.soundboard.flags.console.set(audio::Console::done);
	} else {
		pause_window.open();
		svc.soundboard.flags.console.set(audio::Console::menu_open);
	}
	svc.ticker.paused() ? svc.ticker.unpause() : svc.ticker.pause();
}

void Dojo::bake_maps(ServiceProvider& svc, std::vector<int> ids, bool current) {
	for (auto& id : ids) {
		if (id == 0) { continue; } // intro
		gui_map.clear();
		if (svc.data.room_discovered(id)) {
			inventory_window.minimap.bake(svc, gui_map, id, current);
		} else {
			inventory_window.minimap.bake(svc, gui_map, id, current, true);
		}
	}
}

} // namespace automa
