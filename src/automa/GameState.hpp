//
//  GameState.hpp
//  automa
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <SFML/Graphics.hpp>
#include <cstdio>
#include <memory>
#include <chrono>
#include "../level/Map.hpp"
#include "../components/PhysicsComponent.hpp"
#include "../entities/player/Player.hpp"
#include "../setup/LookupTables.hpp"
#include "../gui/HUD.hpp"
#include "../graphics/Background.hpp"

namespace automa {

	//globals

	enum class STATE {
		STATE_NULL,
		STATE_INIT,
		STATE_EXIT,
		STATE_MENU,
		STATE_OPTIONS,
		STATE_FILE,
		STATE_MAIN,
		STATE_DOJO
	};

	class GameState {

	public:

		using Time = std::chrono::duration<float>;

		GameState() = default;
		GameState(int id) {

		}
		GameState& operator=(GameState&&) = delete;

		template<typename T> class StateID;

		virtual void init(const std::string& load_path) {};
		virtual void setTilesetTexture(sf::Texture& t) {};
		virtual void handle_events(sf::Event& event) {

		};
		virtual void logic() {};
		virtual void render(sf::RenderWindow& win) {


		};

		STATE state = STATE::STATE_NULL;
		bool debug_mode{ false };
	};


}

namespace flstates {

	// =======================================================================
	//
	//    MAIN_MENU
	//
	// =======================================================================

	class MainMenu : public automa::GameState {
	public:

		enum class MenuSelection {
			new_game,
			load_game,
			options
		};

		std::unordered_map<MenuSelection, int> menu_selection_id{
			{MenuSelection::new_game, 0}, {MenuSelection::load_game, 1}, {MenuSelection::options, 2}
		};

		//5, 8, 11
		MainMenu() {
			state = automa::STATE::STATE_MENU;
			svc::cameraLocator.get().set_position({ 1, 1 });
		};

		void init(const std::string& load_path) {

			selection_width = 92;
			selection_buffer = 14;
			title_buffer = 70;
			top_buffer = 296;
			middle = (int)cam::screen_dimensions.x / 2;
			int selection_point = middle - selection_width / 2;

			new_rect = { { middle - 20, top_buffer }, { 40, 10 } };
			load_rect = { { middle - 24, top_buffer + selection_buffer + new_rect.getSize().y }, { 50, 16 } };
			options_rect = { { middle - 46, top_buffer + (selection_buffer * 2) + new_rect.getSize().y + load_rect.getSize().y}, { 92, 22 } };

			selection = MenuSelection::new_game;
			left_dot = vfx::Attractor({ new_rect.getPosition().x - dot_pad.x, new_rect.getPosition().y + dot_pad.y }, flcolor::bright_orange, 0.008f);
			right_dot = vfx::Attractor({ new_rect.getPosition().x + new_rect.width + dot_pad.x, new_rect.getPosition().y + dot_pad.y }, flcolor::bright_orange, 0.008f);

			left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
			left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);
			right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
			right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);

			left_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(new_rect.getPosition()));
			right_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(new_rect.getPosition() + new_rect.getSize()));
			left_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(new_rect.getPosition()));
			right_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(new_rect.getPosition() + new_rect.getSize()));


			int y_height_counter{ 0 };
			for (auto i = 0; i < 6; ++i) {

				//the menu options have different sprite heights
				int height{};
				switch (i % 3) {
				case 0: height = 10; break;
				case 1: height = 16; break;
				case 2: height = 22; break;
				}

				title_assets.push_back(sf::Sprite{ svc::assetLocator.get().t_title_assets, sf::IntRect({0, y_height_counter}, {selection_width, height }) });

				switch (i % 3) {
				case 0: title_assets.at(i).setPosition(selection_point, new_rect.getPosition().y); break;
				case 1: title_assets.at(i).setPosition(selection_point, load_rect.getPosition().y); break;
				case 2: title_assets.at(i).setPosition(selection_point, options_rect.getPosition().y); break;
				}

				y_height_counter += height;

			}
			
			title = sf::Sprite{ svc::assetLocator.get().t_title, sf::IntRect({ 0, 0 }, { (int)cam::screen_dimensions.x, (int)cam::screen_dimensions.y }) };

		}

		void setTilesetTexture(sf::Texture& t) {}

		void handle_events(sf::Event& event) {

			if (event.type == sf::Event::EventType::KeyPressed) {
				if(event.key.code == sf::Keyboard::Down) {
					selection = (menu_selection_id.at(selection) % 3 == 2) ? MenuSelection::new_game : (MenuSelection)(menu_selection_id.at(selection) + 1);
				}
				if (event.key.code == sf::Keyboard::Up) {
					selection = (menu_selection_id.at(selection) % 3 == 0) ? MenuSelection::options : (MenuSelection)(menu_selection_id.at(selection) - 1);
				}
				if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Enter) {
					if (selection == MenuSelection::new_game) {

						svc::dataLocator.get().load_blank_save();
						svc::stateControllerLocator.get().save_loaded = true;
					}
					if (selection == MenuSelection::load_game) {

						svc::stateControllerLocator.get().submenu = automa::menu_type::file_select;
						svc::stateControllerLocator.get().trigger_submenu = true;

					}
					if (selection == MenuSelection::options) {
						//todo: make options menu

					}
				}
			}

		}

		void logic() {
			left_dot.update();
			right_dot.update();
			switch (selection) {
			case MenuSelection::new_game:
				dot_pad.y = 5.f;
				left_dot.set_target_position({ new_rect.getPosition().x - dot_pad.x, new_rect.getPosition().y + dot_pad.y });
				right_dot.set_target_position({ new_rect.getPosition().x + new_rect.width + dot_pad.x, new_rect.getPosition().y + dot_pad.y });
				break;
			case MenuSelection::load_game:
				dot_pad.y = 8.f;
				left_dot.set_target_position({ load_rect.getPosition().x - dot_pad.x, load_rect.getPosition().y + dot_pad.y });
				right_dot.set_target_position({ load_rect.getPosition().x + load_rect.width + dot_pad.x, load_rect.getPosition().y + dot_pad.y });
				break;
			case MenuSelection::options:
				dot_pad.y = 11.f;
				left_dot.set_target_position({ options_rect.getPosition().x - dot_pad.x, options_rect.getPosition().y + dot_pad.y });
				right_dot.set_target_position({ options_rect.getPosition().x + options_rect.width + dot_pad.x, options_rect.getPosition().y + dot_pad.y });
				break;
			}
		}

		void render(sf::RenderWindow& win) {
			win.draw(title);
			svc::counterLocator.get().at(svc::draw_calls)++;

			int selection_adjustment{};
			for (auto i = 0; i < 3; ++i) {
				if (i == menu_selection_id.at(selection)) { selection_adjustment = 3; } else { selection_adjustment = 0; }
				if (i + selection_adjustment < 6) {
					win.draw(title_assets.at(i + selection_adjustment));
					svc::counterLocator.get().at(svc::draw_calls)++;
				}
			}

			left_dot.render(win, { 0, 0 });
			right_dot.render(win, { 0, 0 });

		}

		//title textures
		sf::Sprite title{};
		std::vector<sf::Sprite> title_assets{};

		MenuSelection selection{};

		int selection_width{};
		int selection_buffer{};
		int title_buffer{};
		int top_buffer{};
		int middle{};

		sf::IntRect new_rect{};
		sf::IntRect load_rect{};
		sf::IntRect options_rect{};

		vfx::Attractor left_dot{};
		vfx::Attractor right_dot{};
		sf::Vector2<float> dot_pad{ 24.f, 8.f };

	};

	// =======================================================================
	//
	//    FILE_MENU
	//
	// =======================================================================

	class FileMenu : public automa::GameState {

		static const int num_files{ 3 };

		public:

		std::array<int, num_files> files {
			1, 2, 3
		};

		FileMenu() {
			state = automa::STATE::STATE_FILE;
			svc::cameraLocator.get().set_position({ 1, 1 });
			init("");
		};

		void init(const std::string& load_path) {

			title.setPosition(0, 0);
			title.setSize(static_cast<sf::Vector2f>(cam::screen_dimensions));
			title.setFillColor(flcolor::ui_black);

			selection_width = 92;
			selection_buffer = 14;
			top_buffer = 186;
			middle = (int)cam::screen_dimensions.x / 2;
			int selection_point = middle - selection_width / 2;
			text_left = middle - text_dim.x / 2;
			text_right = middle + text_dim.x / 2;

			for(int i = 0; i < num_files; ++i) {
				file_rects.at(i) = sf::IntRect({ text_left, top_buffer + (text_dim.y * (i)) + (selection_buffer * (i % num_files)) }, text_dim);
			}

			left_dot = vfx::Attractor({ file_rects.at(0).getPosition().x - dot_pad.x, file_rects.at(0).getPosition().y + dot_pad.y }, flcolor::bright_orange, 0.008f);
			right_dot = vfx::Attractor({ file_rects.at(0).getPosition().x + file_rects.at(0).width + dot_pad.x, file_rects.at(0).getPosition().y + dot_pad.y }, flcolor::bright_orange, 0.008f);

			left_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
			left_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);
			right_dot.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.83f, 0.83f}, 1.0f);
			right_dot.collider.physics.maximum_velocity = sf::Vector2<float>(4.5f, 4.5f);

			left_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
			right_dot.collider.bounding_box.set_position(static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));
			left_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition()));
			right_dot.collider.physics.position = (static_cast<sf::Vector2<float>>(file_rects.at(0).getPosition() + file_rects.at(0).getSize()));

			for (auto i = 0; i < num_files * 2; ++i) {

				file_text.at(i) = sf::Sprite{ svc::assetLocator.get().t_file_text, sf::IntRect({0, i * text_dim.y}, text_dim) };
				file_text.at(i).setPosition(text_left, top_buffer + (text_dim.y * (i % num_files)) + (selection_buffer * (i % num_files)));

			}

		}

		void setTilesetTexture(sf::Texture& t) {}

		void handle_events(sf::Event& event) {

			if (event.type == sf::Event::EventType::KeyPressed) {
				if (event.key.code == sf::Keyboard::Down) {
					++selection;
				}
				if (event.key.code == sf::Keyboard::Up) {
					--selection;
				}
				if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Enter) {
					//constrain selection
					if (selection >= num_files) { selection = 0; }
					if (selection < 0) { selection = num_files - 1; }
					svc::dataLocator.get().load_progress(selection);
					svc::stateControllerLocator.get().save_loaded = true;
				}
			}

		}

		void logic() {
			//constrain selection
			if (selection >= num_files) { selection = 0; }
			if (selection < 0) { selection = num_files - 1; }
			left_dot.update();
			right_dot.update();
			left_dot.set_target_position({ text_left - dot_pad.x, file_rects.at(selection).getPosition().y + dot_pad.y });
			right_dot.set_target_position({ text_right + dot_pad.x, file_rects.at(selection).getPosition().y + dot_pad.y });
		}

		void render(sf::RenderWindow& win) {
			win.draw(title);
			svc::counterLocator.get().at(svc::draw_calls)++;

			int selection_adjustment{};
			for (auto i = 0; i < num_files; ++i) {
				if (i == selection) { selection_adjustment = 3; } else { selection_adjustment = 0; }
				win.draw(file_text.at(i + selection_adjustment));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}

			left_dot.render(win, { 0, 0 });
			right_dot.render(win, { 0, 0 });

		}

		//menu textures
		sf::RectangleShape title{};
		std::array<sf::Sprite, num_files * 2> file_text{};

		

		int selection{ 0 };

		int selection_width{};
		int selection_buffer{};
		int top_buffer{};
		int middle{};
		int text_left{};
		int text_right{};
		sf::Vector2i text_dim{ 72, 16 };

		std::array<sf::IntRect, num_files> file_rects{};

		vfx::Attractor left_dot{};
		vfx::Attractor right_dot{};
		sf::Vector2<float> dot_pad{ 24.f, 8.f };

	};

	// =======================================================================
	//
	//    DOJO
	//
	// =======================================================================

	class Dojo : public automa::GameState {
	public:

		Dojo() {
			state = automa::STATE::STATE_DOJO;
			svc::cameraLocator.get().set_position({ 1, 1 });
			svc::playerLocator.get().set_position({ 360, 500 });
		}
		void init(const std::string& load_path) {
			map.load(load_path);
			svc::playerLocator.get().behavior.current_state = behavior::Behavior(behavior::idle);
			svc::playerLocator.get().flags.input.reset(Input::inspecting);
			tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(map.style));
			for (int i = 0; i < 16; ++i) {
				for (int j = 0; j < 16; ++j) {
					tileset_sprites.push_back(sf::Sprite());
					tileset_sprites.back().setTexture(tileset);
					tileset_sprites.back().setTextureRect(sf::IntRect({ j * TILE_WIDTH, i * TILE_WIDTH }, { TILE_WIDTH, TILE_WIDTH }));


					svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({ 0, 0 }, { 128, 256 }));
					svc::assetLocator.get().sp_ui_test.setTextureRect(sf::IntRect({ 0, 0 }, { 420, 128 }));
				}
			}
			svc::playerLocator.get().collider.physics.zero();
			svc::playerLocator.get().flags.state.set(State::alive);
			bool found_one = false;
			//only search for door entry if room was not loaded from main menu
			if (!svc::stateControllerLocator.get().save_loaded) {
				for (auto& portal : map.portals) {
					if (portal.destination_map_id == svc::stateControllerLocator.get().source_id) {
						found_one = true;
						sf::Vector2<float> spawn_position{ portal.position.x + std::floor(portal.dimensions.x / 2), portal.position.y + portal.dimensions.y - PLAYER_HEIGHT };
						svc::playerLocator.get().set_position(spawn_position);
						svc::cameraLocator.get().center(spawn_position);
						svc::cameraLocator.get().physics.position = spawn_position - sf::Vector2<float>(svc::cameraLocator.get().bounding_box.width / 2, svc::cameraLocator.get().bounding_box.height / 2);
					}
				}
			}
			if (!found_one) {
				float ppx = svc::dataLocator.get().save["player_data"]["position"]["x"].as<float>();
				float ppy = svc::dataLocator.get().save["player_data"]["position"]["y"].as<float>();
				sf::Vector2f player_pos = { ppx, ppy };
				svc::playerLocator.get().set_position(player_pos);
			}
			//save was loaded from a json, so we successfully skipped door search
			svc::stateControllerLocator.get().save_loaded = false;

			//        svc::assetLocator.get().abandoned.setVolume(50);
			//        svc::assetLocator.get().abandoned.play();
			//        svc::assetLocator.get().abandoned.setLoop(true);
					/*
					svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
					svc::assetLocator.get().three_pipes.play();
					svc::assetLocator.get().three_pipes.setLoop(true);
					*/
					/*svc::assetLocator.get().brown_noise.setVolume(20);
					svc::assetLocator.get().brown_noise.play();
					svc::assetLocator.get().brown_noise.setLoop(true);*/

		}

		void setTilesetTexture(sf::Texture& t) {
			tileset_sprites.clear();
			for (int i = 0; i < 16; ++i) {
				for (int j = 0; j < 16; ++j) {
					tileset_sprites.push_back(sf::Sprite());
					tileset_sprites.back().setTexture(t);
					tileset_sprites.back().setTextureRect(sf::IntRect({ j * TILE_WIDTH, i * TILE_WIDTH }, { TILE_WIDTH, TILE_WIDTH }));
				}
			}
		}

		void handle_events(sf::Event& event) {
			if (event.type == sf::Event::EventType::KeyPressed) {
				svc::inputStateLocator.get().handle_press(event.key.code);
				if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right) {
					svc::playerLocator.get().flags.input.set(Input::exit_request);
				}
			}
			if (event.type == sf::Event::EventType::KeyReleased) {
				svc::inputStateLocator.get().handle_release(event.key.code);
				if (event.key.code == sf::Keyboard::Z || event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right) {
					svc::playerLocator.get().flags.input.reset(Input::exit_request);
					svc::playerLocator.get().unrestrict_inputs();
					svc::playerLocator.get().flags.input.reset(Input::inspecting);
					svc::playerLocator.get().flags.input.reset(Input::inspecting_trigger);
				}
			}
			svc::playerLocator.get().handle_events(event);
			if (event.type == sf::Event::EventType::KeyPressed) {
				if (event.key.code == sf::Keyboard::H) {
					svc::globalBitFlagsLocator.get().set(svc::global_flags::greyblock_state);
					svc::globalBitFlagsLocator.get().set(svc::global_flags::greyblock_trigger);
				}
			}
			if (event.type == sf::Event::EventType::KeyPressed) {
				if (event.key.code == sf::Keyboard::LControl) {
					map.show_minimap = !map.show_minimap;
				}
			}
			if (event.type == sf::Event::EventType::KeyPressed) {
				if (event.key.code == sf::Keyboard::B) {
					x++;
					if (x % 4 == 0) { x = 0; }
					svc::assetLocator.get().sp_bryn_test.setTextureRect(sf::IntRect({ x * 128, 0 }, { 128, 256 }));
				}
			}
		}

		void logic() {
			svc::cameraLocator.get().previous_position = svc::cameraLocator.get().position;
			map.update();
			hud.update();
			svc::cameraLocator.get().center(svc::playerLocator.get().anchor_point);
			svc::cameraLocator.get().update();
			svc::cameraLocator.get().restrict_movement(map.real_dimensions);
			if (map.real_dimensions.x < cam::screen_dimensions.x) { svc::cameraLocator.get().fix_vertically(map.real_dimensions); }
			if (map.real_dimensions.y < cam::screen_dimensions.y) { svc::cameraLocator.get().fix_horizontally(map.real_dimensions); }
			svc::playerLocator.get().update(svc::clockLocator.get().elapsed_time);
			for (auto& critter : map.critters) { critter->update(); critter->unique_update(); critter->flags.reset(critter::Flags::shot); }
			svc::assetLocator.get().three_pipes.setVolume(svc::assetLocator.get().music_vol);
			map.debug_mode = debug_mode;

			svc::cameraLocator.get().position = svc::cameraLocator.get().physics.position;
			svc::cameraLocator.get().observed_velocity.x = svc::cameraLocator.get().position.x - svc::cameraLocator.get().previous_position.x;
			svc::cameraLocator.get().observed_velocity.y = svc::cameraLocator.get().position.y - svc::cameraLocator.get().previous_position.y;
		}

		void render(sf::RenderWindow& win) {
			sf::Vector2<float> camvel = svc::cameraLocator.get().physics.velocity;
			sf::Vector2<float> camoffset = svc::cameraLocator.get().physics.position + camvel;
			map.render_background(win, tileset_sprites, svc::cameraLocator.get().physics.position);



			map.render(win, tileset_sprites, svc::cameraLocator.get().physics.position);
			if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) { svc::playerLocator.get().collider.render(win, svc::cameraLocator.get().physics.position); }
			hud.render(win);

			map.render_console(win);

			svc::assetLocator.get().sp_ui_test.setPosition(20, cam::screen_dimensions.y - 148);
			svc::assetLocator.get().sp_bryn_test.setPosition(20, cam::screen_dimensions.y - 276);
			//        win.draw(svc::assetLocator.get().sp_ui_test);
			//        win.draw(svc::assetLocator.get().sp_bryn_test);

			map.transition.render(win);

			if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_trigger)) {
				if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
					tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(lookup::STYLE::PROVISIONAL));
					setTilesetTexture(tileset);
				}
				else {
					tileset = svc::assetLocator.get().tilesets.at(lookup::get_style_id.at(map.style));
					setTilesetTexture(tileset);
				}
			}

		}

		world::Map map{};
		sf::Texture tileset{};
		std::vector<sf::Sprite> tileset_sprites{};
		bool show_colliders{ false };
		int x{ 0 };

		gui::HUD hud{ {20, 20} };

	};

}

/* GameState_hpp */
