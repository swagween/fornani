
#include "Dojo.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id), map(svc, player) {}

void Dojo::init(ServiceProvider& svc, std::string_view room) {

	console = gui::Console(svc);
	player->reset_flags();
	map.load(svc, room);

	// TODO: refactor player initialization
	player->collider.physics.zero();
	player->flags.state.set(player::State::alive);

	bool found_one{};
	// only search for door entry if room was not loaded from main menu
	if (!svc.state_controller.actions.test(Actions::save_loaded)) {
		for (auto& portal : map.portals) {
			if (portal.destination_map_id == svc.state_controller.source_id) {
				found_one = true;
				sf::Vector2<float> spawn_position{portal.position.x + (portal.dimensions.x * 0.5f), portal.position.y + portal.dimensions.y - player->height()};
				player->set_position(spawn_position, true);
				camera.force_center(player->anchor_point);
				if (portal.activate_on_contact) { enter_room.start(90); }
			}
		}
	}
	if (!found_one) {
		float ppx = svc.data.save["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.save["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		player->set_position(player_pos);
	}

	// save was loaded from a json, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);

	player->controller.prevent_movement();
}

void Dojo::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);

	if (event.type == sf::Event::EventType::KeyPressed) {
		svc.controller_map.handle_press(event.key.code);
		if (svc.controller_map.label_to_control.at("menu_toggle").triggered()) { toggle_inventory(); }
	}
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }
	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::LControl) { map.show_minimap = !map.show_minimap; }
	}
}

void Dojo::tick_update(ServiceProvider& svc) {
	enter_room.update();
	if (enter_room.running()) { player->controller.autonomous_walk(); }
	player->update(console, inventory_window);

	map.update(svc, console, inventory_window);

	camera.center(player->anchor_point);
	camera.update(svc);
	camera.restrict_movement(map.real_dimensions);

	map.debug_mode = debug_mode;

	svc.controller_map.reset_triggers();
	player->controller.clean();
	svc.soundboard.play_sounds(svc);
	player->flags.triggers = {};
}

void Dojo::frame_update(ServiceProvider& svc) {
	map.background->update(svc, camera.get_observed_velocity());
	hud.update(*player);
}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {

	hud.set_corner_pad(svc, false); // reset hud position to corner
	map.render_background(svc, win, camera.get_position());
	map.render(svc, win, camera.get_position());

	if (!svc.greyblock_mode()) { hud.render(*player, win); }
	inventory_window.render(svc, *player, win);
	map.render_console(svc, console, win);

	map.transition.render(win);
}

void Dojo::toggle_inventory() { inventory_window.active() ? inventory_window.close() : inventory_window.open(); }

} // namespace automa
