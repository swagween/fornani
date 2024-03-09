
#include "Dojo.hpp"

namespace automa {

Dojo::Dojo() {
	state = STATE::STATE_DOJO;
	svc::cameraLocator.get().set_position({1, 1});
	svc::playerLocator.get().set_position({360, 500});
}
void Dojo::init(std::string const& load_path) {

	hud.set_corner_pad(false); // reset hud position to corner
	svc::playerLocator.get().reset_flags();

	map.load(load_path);
	tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(map.style));
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			tileset_sprites.push_back(sf::Sprite());
			tileset_sprites.back().setTexture(tileset);
			tileset_sprites.back().setTextureRect(sf::IntRect({j * asset::TILE_WIDTH, i * asset::TILE_WIDTH}, {asset::TILE_WIDTH, asset::TILE_WIDTH}));

			svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({0, 0}, {128, 256}));
			svc::assetLocator.get().sp_ui_test.setTextureRect(sf::IntRect({0, 0}, {420, 128}));
		}
	}

	// TODO: refactor player initialization
	svc::playerLocator.get().collider.physics.zero();
	svc::playerLocator.get().flags.state.set(player::State::alive);

	bool found_one = false;
	// only search for door entry if room was not loaded from main menu
	if (!svc::stateControllerLocator.get().save_loaded) {
		for (auto& portal : map.portals) {
			if (portal.destination_map_id == svc::stateControllerLocator.get().source_id) {
				found_one = true;
				sf::Vector2<float> spawn_position{portal.position.x + std::floor(portal.dimensions.x / 2), portal.position.y + portal.dimensions.y - player::PLAYER_HEIGHT};
				svc::playerLocator.get().set_position(spawn_position);
				svc::cameraLocator.get().center(spawn_position);
				svc::cameraLocator.get().physics.position = spawn_position - sf::Vector2<float>(svc::cameraLocator.get().bounding_box.width / 2, svc::cameraLocator.get().bounding_box.height / 2);
			}
		}
	}
	if (!found_one) {
		float ppx = svc::dataLocator.get().save["player_data"]["position"]["x"].as<float>();
		float ppy = svc::dataLocator.get().save["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		svc::playerLocator.get().set_position(player_pos);
	}

	// save was loaded from a json, so we successfully skipped door search
	svc::stateControllerLocator.get().save_loaded = false;

	svc::inputStateLocator.get().reset_triggers();
	svc::playerLocator.get().controller = {};
	svc::playerLocator.get().controller.prevent_movement();
}

void Dojo::setTilesetTexture(sf::Texture& t) {
	tileset_sprites.clear();
	for (int i = 0; i < 16; ++i) {
		for (int j = 0; j < 16; ++j) {
			tileset_sprites.push_back(sf::Sprite());
			tileset_sprites.back().setTexture(t);
			tileset_sprites.back().setTextureRect(sf::IntRect({j * asset::TILE_WIDTH, i * asset::TILE_WIDTH}, {asset::TILE_WIDTH, asset::TILE_WIDTH}));
		}
	}
}

void Dojo::handle_events(sf::Event& event) {
	if (event.type == sf::Event::EventType::KeyPressed) { svc::inputStateLocator.get().handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc::inputStateLocator.get().handle_release(event.key.code); }
	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::LControl) { map.show_minimap = !map.show_minimap; }
	}
	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::B) {
			x++;
			if (x % 4 == 0) { x = 0; }
			svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({x * 128, 0}, {128, 256}));
		}
	}
}

void Dojo::tick_update() {
	svc::playerLocator.get().update();

	map.update();
	svc::cameraLocator.get().center(svc::playerLocator.get().anchor_point);
	svc::cameraLocator.get().update();
	svc::cameraLocator.get().restrict_movement(map.real_dimensions);
	if (map.real_dimensions.x < cam::screen_dimensions.x) { svc::cameraLocator.get().fix_vertically(map.real_dimensions); }
	if (map.real_dimensions.y < cam::screen_dimensions.y) { svc::cameraLocator.get().fix_horizontally(map.real_dimensions); }
	for (auto& critter : map.critters) {
		critter->update();
		critter->unique_update();
		critter->flags.reset(critter::Flags::shot);
	}

	svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
	map.debug_mode = debug_mode;

	svc::inputStateLocator.get().reset_triggers();
	svc::playerLocator.get().controller.clean();

}

void Dojo::frame_update() {
	map.background->update();
	hud.update();
}

void Dojo::render(sf::RenderWindow& win) {
	sf::Vector2<float> camvel = svc::cameraLocator.get().physics.velocity;
	sf::Vector2<float> camoffset = svc::cameraLocator.get().physics.position + camvel;
	map.render_background(win, tileset_sprites, svc::cameraLocator.get().physics.position);

	map.render(win, tileset_sprites, svc::cameraLocator.get().physics.position);
	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
		svc::playerLocator.get().collider.render(win, svc::cameraLocator.get().physics.position);
	}

	if (!svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) { hud.render(win); }

	map.render_console(win);

	svc::assetLocator.get().sp_ui_test.setPosition(20, cam::screen_dimensions.y - 148);
	svc::assetLocator.get().sp_bryn_test.setPosition(20, cam::screen_dimensions.y - 276);

	map.transition.render(win);


	if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_trigger)) {
		if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
			tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(lookup::STYLE::PROVISIONAL));
			setTilesetTexture(tileset);
		} else {
			tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(map.style));
			setTilesetTexture(tileset);
		}
	}
}

} // namespace automa
