
#include "Dojo.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

Dojo::Dojo(ServiceProvider& svc, player::Player& player, int id) : GameState(svc, player, id), map(svc, player) {}

void Dojo::init(ServiceProvider& svc, std::string_view room) {

	console = gui::Console(svc);

	hud.set_corner_pad(svc, false); // reset hud position to corner
	player->reset_flags();

	map.load(svc, room);
	tileset = svc.assets.tilesets.at(map.style_id);
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			tileset_sprites.push_back(sf::Sprite());
			tileset_sprites.back().setTexture(tileset);
			tileset_sprites.back().setTextureRect(sf::IntRect({j * asset::TILE_WIDTH, i * asset::TILE_WIDTH}, {asset::TILE_WIDTH, asset::TILE_WIDTH}));

			svc.assets.sp_bryn_test.setTextureRect(sf::IntRect({0, 0}, {128, 256}));
			svc.assets.sp_ui_test.setTextureRect(sf::IntRect({0, 0}, {420, 128}));
		}
	}

	// TODO: refactor player initialization
	player->collider.physics.zero();
	player->flags.state.set(player::State::alive);

	bool found_one = false;
	// only search for door entry if room was not loaded from main menu
	if (!svc.state_controller.actions.test(Actions::save_loaded)) {
		for (auto& portal : map.portals) {
			if (portal.destination_map_id == svc.state_controller.source_id) {
				found_one = true;
				sf::Vector2<float> spawn_position{portal.position.x + std::floor(portal.dimensions.x / 2), portal.position.y + portal.dimensions.y - player::PLAYER_HEIGHT};
				player->set_position(spawn_position);
				svc::cameraLocator.get().center(svc, spawn_position);
				svc::cameraLocator.get().physics.position = spawn_position - sf::Vector2<float>(svc::cameraLocator.get().bounding_box.width / 2, svc::cameraLocator.get().bounding_box.height / 2);
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

	player->controller = {};
	player->controller.prevent_movement();
}

void Dojo::setTilesetTexture(ServiceProvider& svc, sf::Texture& t) {
	tileset_sprites.clear();
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			tileset_sprites.push_back(sf::Sprite());
			tileset_sprites.back().setTexture(t);
			tileset_sprites.back().setTextureRect(sf::IntRect({j * asset::TILE_WIDTH, i * asset::TILE_WIDTH}, {asset::TILE_WIDTH, asset::TILE_WIDTH}));
		}
	}
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
	player->update(console, inventory_window);

	map.update(svc, console, inventory_window);

	svc::cameraLocator.get().center(svc, player->anchor_point);
	svc::cameraLocator.get().update(svc);
	svc::cameraLocator.get().restrict_movement(map.real_dimensions);
	if (map.real_dimensions.x < cam::screen_dimensions.x) { svc::cameraLocator.get().fix_vertically(map.real_dimensions); }
	if (map.real_dimensions.y < cam::screen_dimensions.y) { svc::cameraLocator.get().fix_horizontally(map.real_dimensions); }

	map.debug_mode = debug_mode;

	svc.controller_map.reset_triggers();
	player->controller.clean();
	svc.soundboard.play_sounds(svc);
}

void Dojo::frame_update(ServiceProvider& svc) {
	map.background->update(svc);
	hud.update(*player);
}

void Dojo::render(ServiceProvider& svc, sf::RenderWindow& win) {
	sf::Vector2<float> camvel = svc::cameraLocator.get().physics.velocity;
	sf::Vector2<float> camoffset = svc::cameraLocator.get().physics.position + camvel;
	map.render_background(svc, win, svc::cameraLocator.get().physics.position);

	map.render(svc, win, svc::cameraLocator.get().physics.position);

	if (!svc.greyblock_mode()) { hud.render(*player, win); }
	inventory_window.render(svc, *player, win);
	map.render_console(svc, console, win);

	svc.assets.sp_ui_test.setPosition(20, cam::screen_dimensions.y - 148);
	svc.assets.sp_bryn_test.setPosition(20, cam::screen_dimensions.y - 276);

	map.transition.render(win);

	if (svc.debug_flags.test(automa::DebugFlags::greyblock_trigger)) {
		if (svc.greyblock_mode()) {
			tileset = svc.assets.tilesets.at(lookup::get_style_id.at(lookup::STYLE::PROVISIONAL));
			setTilesetTexture(svc, tileset);
		} else {
			tileset = svc.assets.tilesets.at(map.style_id);
			setTilesetTexture(svc, tileset);
		}
	}
}

void Dojo::toggle_inventory() { inventory_window.active() ? inventory_window.close() : inventory_window.open(); }

} // namespace automa
