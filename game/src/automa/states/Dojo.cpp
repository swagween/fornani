
#include "fornani/automa/states/Dojo.hpp"

#include "fornani/service/ServiceProvider.hpp"

#include <tracy/Tracy.hpp>

#include "fornani/utils/Random.hpp"

namespace fornani::automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name) : GameState(svc, player, scene, room_number), map(svc, player, console), gui_map(svc, player, console) {
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
	loading.start();
}

void Dojo::tick_update(ServiceProvider& svc) {
	svc.a11y.set_action_ctx_bar_enabled(false);

	loading.is_complete() && !vendor_dialog ? svc.app_flags.set(AppFlags::in_game) : svc.app_flags.reset(AppFlags::in_game);
	loading.update();

	svc.soundboard.play_sounds(svc, map.get_echo_count(), map.get_echo_rate());

	// set action set
	svc.controller_map.set_action_set(config::ActionSet::Platformer);
	if (pause_window) { svc.controller_map.set_action_set(config::ActionSet::Menu); }
	if (inventory_window) { svc.controller_map.set_action_set(config::ActionSet::Inventory); }
	if (console.is_active()) { svc.controller_map.set_action_set(config::ActionSet::Menu); }

	console.update(svc);

	if (pause_window) {
		pause_window.value()->update(svc, console);
		if (pause_window.value()->settings_requested()) {
			flags.set(GameStateFlags::settings_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->controls_requested()) {
			flags.set(GameStateFlags::controls_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->exit_requested()) { pause_window = {}; }
		return;
	}

	// TODO: move this somehwere else
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
		}
		return;
	}

	svc.world_clock.update(svc);

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
		util::Random::set_vendor_seed();
		for (auto& vendor : svc.data.marketplace) { vendor.second.generate_inventory(svc); }
		player->visit_history.clear();
	}

	if (inventory_window) {
		inventory_window.value()->update(svc, *player, map);
		if (inventory_window.value()->exit_requested()) { inventory_window = {}; }
	}

	// in-game menus
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_inventory).triggered) { inventory_window = std::make_unique<gui::InventoryWindow>(svc, gui_map); }
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { pause_window = std::make_unique<gui::PauseWindow>(svc); }
	if (svc.controller_map.process_gamepad_disconnection()) { pause_window = std::make_unique<gui::PauseWindow>(svc); }

	enter_room.update();
	if (console.is_complete() && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }
	if (enter_room.running()) { player->controller.autonomous_walk(); }

	player->update(map);
	map.update(svc, console);

	camera.center(player->get_camera_focus_point());
	camera.update(svc);
	camera.restrict_movement(map.real_dimensions);

	map.debug_mode = debug_mode;

	player->controller.clean();
	player->flags.triggers = {};

	map.background->update(svc);
	hud.update(svc, *player);
}

void Dojo::frame_update(ServiceProvider& svc) { ZoneScopedN("Dojo::frame_update"); }

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {
	ZoneScopedN("Dojo::render");
	map.render_background(svc, win, camera.get_position());
	map.render(svc, win, camera.get_position());

	if (!svc.greyblock_mode() && !svc.hide_hud()) { hud.render(*player, win); }
	if (vendor_dialog) { vendor_dialog.value().render(svc, win, *player, map); }
	if (inventory_window) (inventory_window.value()->render(svc, win));
	map.soft_reset.render(win);
	map.transition.render(win);
	if (pause_window) { pause_window.value()->render(svc, win); }
	console.render(win);
	console.write(win);
	player->tutorial.render(win);
	if (svc.debug_mode()) { map.debug(); }
}

void Dojo::bake_maps(ServiceProvider& svc, std::vector<int> ids, bool current) {
	for (auto const& id : ids) {
		if (id == 0) { continue; } // intro
		gui_map.clear();
		if (svc.data.room_discovered(id)) {
			// inventory_window.minimap.bake(svc, gui_map, id, current);
		} else {
			// inventory_window.minimap.bake(svc, gui_map, id, current, true);
		}
	}
}

} // namespace fornani::automa
