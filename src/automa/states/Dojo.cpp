
#include "Dojo.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), map(svc, player, console), gui_map(svc, player, console) {}

void Dojo::init(ServiceProvider& svc, int room_number, std::string room_name) {
	// std::cout << "\n" << room_number;
	// A = shape::Collider({32.f, 32.f});
	// B = shape::Collider({24.f, 24.f});
	// A.stats.GRAV = 0.f;
	// B.stats.GRAV = 0.f;
	// A.physics.position = {200.f, 200.f};

	if (!svc.data.room_discovered(room_number)) {
		svc.data.discovered_rooms.push_back(room_number);
		svc.stats.world.rooms_discovered.update();
	}
	console = gui::Console(svc);
	player->reset_flags();
	// the following should only happen for the editor demo
	if (!svc.data.exists(room_number)) {
		svc.data.rooms.push_back(room_number);
		svc.data.load_data(room_name);
		// std::cout << "Loading New Room...\n";
	}
	map.load(svc, room_number);
	bake_maps(svc, {map.room_id}, true);
	auto m_id = map.room_id;
	bake_maps(svc, svc.data.rooms);
	if (player->has_shield()) { hud.flags.set(gui::HUDState::shield); }
	hud.set_corner_pad(svc, false); // reset hud position to corner
	svc.soundboard.turn_on();

	// TODO: refactor player initialization
	player->collider.physics.zero();

	bool found_one{};
	// only search for door entry if room was not loaded from main menu and player didn't die
	if (!svc.state_controller.actions.test(Actions::save_loaded) && !svc.state_controller.actions.test(Actions::player_death)) {
		for (auto& portal : map.portals) {
			if (portal.get_destination() == svc.state_controller.source_id) {
				found_one = true;
				sf::Vector2<float> spawn_position{portal.position.x + (portal.dimensions.x * 0.5f), portal.position.y + portal.dimensions.y - player->height()};
				player->set_position(spawn_position, true);
				camera.force_center(player->anchor_point);
				if (portal.activate_on_contact()) {
					enter_room.start(90);
				} else {
					if (!portal.already_open()) { portal.close(); }
					player->set_idle();
				}
				if (portal.dimensions.x > 33.f && portal.position.y > 1.f) { player->collider.physics.acceleration.y = -player->physics_stats.jump_velocity; }
			}
		}
	}
	if (!found_one) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		player->set_position(player_pos);
	}

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player->is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }

	player->controller.prevent_movement();
}

void Dojo::handle_events(ServiceProvider& svc, sf::Event& event) {}

void Dojo::tick_update(ServiceProvider& svc) {
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
	// TODO: Split inventorywindow into inventory and map (separate functionality)
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_inventory).triggered || svc.controller_map.digital_action_status(config::DigitalAction::inventory_close).triggered) {
		toggle_inventory(svc);
		if (inventory_window.active() && inventory_window.is_minimap()) { inventory_window.switch_modes(svc); }
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_open_map).triggered || svc.controller_map.digital_action_status(config::DigitalAction::map_close).triggered) {
		// FIXME Opening the map this way breaks the back panel rendering, as it is placed on (0, 0) with default size
		toggle_inventory(svc);
		if (inventory_window.active() && !inventory_window.is_minimap()) { inventory_window.switch_modes(svc); }
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) { toggle_pause_menu(svc); }

	enter_room.update();
	if (console.is_complete() && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }

	if (enter_room.running()) { player->controller.autonomous_walk(); }

	// A.update(svc);
	// B.update(svc);
	// auto mtv = A.bounding_box.testCollisionGetMTV(B.bounding_box, A.bounding_box);
	// if (svc.ticker.every_x_ticks(400)) { std::cout << "MYT x: " << mtv.x << "\n"; }

	player->update(map, console, inventory_window);
	map.update(svc, console, inventory_window);

	if (map.camera_shake()) { camera.begin_shake(); }
	camera.center(player->anchor_point);
	camera.update(svc);
	camera.restrict_movement(map.real_dimensions);

	map.debug_mode = debug_mode;

	player->controller.clean();
	svc.soundboard.play_sounds(svc);
	player->flags.triggers = {};

	pause_window.update(svc, console, true);
	map.background->update(svc, camera.get_observed_velocity());
	console.end_tick();
}

void Dojo::frame_update(ServiceProvider& svc) {
	pause_window.render_update(svc);
	pause_window.clean_off_trigger();
	if (pause_window.active()) { svc.soundboard.play_sounds(svc); }
	hud.update(svc, *player);
}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {

	// B.physics.position = sf::Vector2<float>(sf::Mouse::getPosition());

	map.render_background(svc, win, camera.get_position());
	map.render(svc, win, camera.get_position());

	if (!svc.greyblock_mode()) { hud.render(*player, win); }
	inventory_window.render(svc, *player, win, camera.get_position());
	pause_window.render(svc, *player, win);
	map.transition.render(win);
	map.render_console(svc, console, win);
	player->tutorial.render(win);

	// A.render(win, {});
	// B.render(win, {});
}

void Dojo::toggle_inventory(ServiceProvider& svc) {
	if (pause_window.active()) { return; }

	if (inventory_window.active()) {
		svc.soundboard.flags.console.set(audio::Console::done);
		inventory_window.close();
	} else {
		inventory_window.minimap.update(svc, map, *player);
		inventory_window.open();
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
		svc.soundboard.play_sounds(svc);
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
