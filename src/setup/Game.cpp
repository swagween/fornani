#include "Game.hpp"
#include <steam/steam_api.h>
#include <ctime>
#include "../gui/ActionContextBar.hpp"
#include "WindowManager.hpp"

namespace fornani {

Game::Game(char** argv, WindowManager& window) : services(argv), player(services) {
	services.stopwatch.start();
	services.window = &window;
	services.constants.screen_dimensions = window.screen_dimensions;
	// controls
	services.data.load_controls(services.controller_map);
	services.data.load_settings();
	// text
	services.text.finder.setResourcePath(argv);
	services.text.load_data();
	services.debug_text.setFont(services.text.fonts.basic);
	services.debug_text.setCharacterSize(16);
	services.debug_text.setFillColor(sf::Color::White);
	// image
	services.assets.finder.setResourcePath(argv);
	services.assets.import_textures();
	// sounds
	services.music.finder.setResourcePath(argv);
	services.assets.load_audio();
	playtest.m_musicplayer = true;
	services.music.turn_on();
	// player
	player.init(services);

	// state manager
	game_state.set_current_state(std::make_unique<automa::MainMenu>(services, player, "main"));
	game_state.get_current_state().init(services, 100);

	background.setSize(static_cast<sf::Vector2<float>>(services.constants.screen_dimensions));
	background.setPosition(0, 0);
	background.setFillColor(services.styles.colors.ui_black);
}

void Game::run(bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2<float> player_position) {
	if (services.window->fullscreen()) { services.app_flags.set(automa::AppFlags::fullscreen); }

	measurements.win_size.x = services.window->get().getSize().x;
	measurements.win_size.y = services.window->get().getSize().y;

	// for editor demo. should be excluded for releases.
	if (demo) {
		services.debug_flags.set(automa::DebugFlags::demo_mode);
		flags.set(GameFlags::in_game);
		game_state.get_current_state().target_folder.paths.scene = levelpath;
		services.music.turn_off();
		services.data.load_progress(player, 0);
		game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
		game_state.get_current_state().init(services, room_id, levelpath.filename().string());
		services.state_controller.demo_level = room_id;
		std::cout << "Launching demo in room " << room_id << "from folder " << levelpath.filename() << "\n";
		services.state_controller.player_position = player_position;
		player.set_position(player_position);
	} else {
		game_state.get_current_state().target_folder.paths.scene = services.data.finder.scene_path;
		game_state.get_current_state().target_folder.paths.region = services.data.finder.scene_path + "/firstwind";
	}

	gui::ActionContextBar ctx_bar(services);

	std::cout << "> Success\n";
	services.stopwatch.stop();
	services.stopwatch.print_time();
	// game loop
	sf::Clock deltaClock{};

	while (services.window->get().isOpen()) {

		auto smp = services.random.percent_chance(10) ? 1 : 0;
		rng_test.sample += smp;
		++rng_test.total;

		if (services.state_controller.actions.test(automa::Actions::shutdown)) { break; }
		if (services.death_mode()) { flags.reset(GameFlags::in_game); }

		services.ticker.start_frame();

		// SFML event variable
		auto event = sf::Event{};

		bool valid_event{true};
		// check window events
		while (services.window->get().pollEvent(event)) {
			player.animation.state = {};
			switch (event.type) {
			case sf::Event::Closed: goto shutdown;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::F2) { valid_event = false; }
				if (event.key.code == sf::Keyboard::F3) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Slash) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Unknown) { valid_event = false; }
				if (event.key.code == sf::Keyboard::D) {
					// debug() ? services.debug_flags.reset(automa::DebugFlags::imgui_overlay) : services.debug_flags.set(automa::DebugFlags::imgui_overlay);
					// services.assets.sharp_click.play();
					// services.state_controller.actions.set(automa::Actions::print_stats);
				}
				if (event.key.code == sf::Keyboard::Q) {
					// game_state.set_current_state(std::make_unique<automa::MainMenu>(services, player, "main"));
					// flags.set(GameFlags::in_game);
				}
				if (event.key.code == sf::Keyboard::P) {
					services.toggle_debug();
					if (flags.test(GameFlags::playtest)) {
						flags.reset(GameFlags::playtest);
						services.soundboard.flags.menu.set(audio::Menu::forward_switch);
					} else {
						flags.set(GameFlags::playtest);
						services.soundboard.flags.menu.set(audio::Menu::backward_switch);
					}
				}
				if (event.key.code == sf::Keyboard::Equal) { take_screenshot(services.window->screencap); }
				if (event.key.code == sf::Keyboard::H) {
					// services.debug_flags.set(automa::DebugFlags::greyblock_trigger);
					// services.debug_flags.test(automa::DebugFlags::greyblock_mode) ? services.debug_flags.reset(automa::DebugFlags::greyblock_mode) : services.debug_flags.set(automa::DebugFlags::greyblock_mode);
				}
				break;
			case sf::Event::KeyReleased:
				if (event.key.code == sf::Keyboard::F2) { valid_event = false; }
				if (event.key.code == sf::Keyboard::F3) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Slash) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Unknown) { valid_event = false; }
				break;
			default: break;
			}
			game_state.get_current_state().handle_events(services, event);
			services.controller_map.handle_event(event);
			if (valid_event) { ImGui::SFML::ProcessEvent(event); }
			valid_event = true;
		}

		SteamAPI_RunCallbacks();

		services.stopwatch.start();
		// game logic and rendering
		services.music.update();
		bool has_focus = services.window->get().hasFocus();
		services.ticker.tick([this, has_focus, &ctx_bar = ctx_bar, &services = services] {
			services.controller_map.update();
			game_state.get_current_state().tick_update(services);
			if (services.a11y.is_action_ctx_bar_enabled()) { ctx_bar.update(services); }
		});
		game_state.get_current_state().frame_update(services);
		game_state.process_state(services, player, *this);
		if (services.state_controller.actions.consume(automa::Actions::screenshot)) { take_screenshot(services.window->screencap); }

		services.stopwatch.stop();

		ImGui::SFML::Update(services.window->get(), deltaClock.restart());

		// ImGui stuff
		if (flags.test(GameFlags::playtest)) { playtester_portal(services.window->get()); }

		// my renders
		services.window->get().clear();
		services.window->get().draw(background);

		game_state.get_current_state().render(services, services.window->get());

		if (services.a11y.is_action_ctx_bar_enabled()) { ctx_bar.render(services.window->get()); }

		ImGui::SFML::Render(services.window->get());
		services.window->get().display();

		services.ticker.end_frame();
	}

shutdown:
	// shutdown
	// explicitly delete music player since it can't be deleted after AssetManager
	services.music.stop();
	ImGui::SFML::Shutdown();
}

void Game::playtester_portal(sf::RenderWindow& window) {
	if (!flags.test(GameFlags::playtest)) { return; }
	// if (flags.test(GameFlags::in_game)) { return; }
	bool* b_debug{};
	float const PAD = 10.0f;
	static int corner = 1;
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	if (corner != -1) {
		ImGuiViewport const* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
		window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
		window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		window_flags |= ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowBgAlpha(0.65f); // Transparent background
		if (ImGui::Begin("Playtester Portal", b_debug, window_flags)) {
			ImGui::Text("Playtester Portal");
			ImGui::Separator();
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
				if (ImGui::BeginTabItem("General")) {
					ImGui::Text("Region: %s", game_state.get_current_state().target_folder.paths.region.string().c_str());
					ImGui::Text("Room: %s", game_state.get_current_state().target_folder.paths.room.string().c_str());
					ImGui::Text("demo mode: %s", services.demo_mode() ? "Enabled" : "Disabled");
					if (ImGui::Button("Toggle Demo Mode")) { services.debug_flags.test(automa::DebugFlags::demo_mode) ? services.debug_flags.reset(automa::DebugFlags::demo_mode) : services.debug_flags.set(automa::DebugFlags::demo_mode); }
					if (ImGui::Button("Toggle Greyblock Mode")) {
						services.debug_flags.set(automa::DebugFlags::greyblock_trigger);
						services.debug_flags.test(automa::DebugFlags::greyblock_mode) ? services.debug_flags.reset(automa::DebugFlags::greyblock_mode) : services.debug_flags.set(automa::DebugFlags::greyblock_mode);
					}
					ImGui::Text("Active Projectiles: %i", services.map_debug.active_projectiles);
					ImGui::Text("32t max: %u", static_cast<unsigned int>(std::numeric_limits<uint32_t>::max()));
					ImGui::Separator();
					ImGui::Text("Player");
					ImGui::Separator();
					ImGui::Text("World Grounded? %s", player.collider.perma_grounded() ? "Yes" : "No");
					ImGui::Text("Horizontal Movement: %f", player.controller.horizontal_movement());
					ImGui::Text("Coyote Time: %i", player.controller.get_jump().get_coyote());
					ImGui::Text("Push Time: %i", player.cooldowns.push.get_cooldown());
					ImGui::Separator();
					ImGui::Text("Jump");
					ImGui::Separator();
					ImGui::Text("Jumping? %s", player.collider.jumping() ? "Yes" : "No");
					ImGui::Text("Can Jump? %s", player.controller.can_jump() ? "Yes" : "No");
					ImGui::Text("Jump Count: %i", player.controller.get_jump().get_count());
					ImGui::Text("Jump Request: %i", player.controller.get_jump().get_request());
					ImGui::Text("Downhill? %s", player.collider.downhill() ? "Yes" : "No");
					ImGui::Text("Wallsliding? %s", player.controller.get_wallslide().is_wallsliding() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("X Position: %.2f", player.collider.physics.position.x / 32.f);
					ImGui::Text("Y Position: %.2f", player.collider.physics.position.y / 32.f);
					ImGui::Text("X Velocity: %.2f", player.collider.physics.velocity.x);
					ImGui::Text("Y Velocity: %.2f", player.collider.physics.velocity.y);
					ImGui::Text("Apparent X Velocity: %.2f", player.collider.physics.apparent_velocity().x);
					ImGui::Text("Apparent Y Velocity: %.2f", player.collider.physics.apparent_velocity().y);
					ImGui::Separator();
					ImGui::Text("Inventory Size: %i", static_cast<int>(player.catalog.categories.inventory.items.size()));
					ImGui::Text("Visit History: ");
					for (auto& room : player.visit_history.rooms_visited) {
						ImGui::Text("%i, ", room);
						ImGui::SameLine();
					}

					ImGui::Separator();
					ImGui::Text("Ticker");
					ImGui::Text("dt: %.8f", services.ticker.dt.count());
					ImGui::Separator();
					ImGui::Text("Seconds Passed: %.2f", services.ticker.total_seconds_passed.count());
					ImGui::Text("Seconds Passed In-Game: %.2f", services.ticker.in_game_seconds_passed.count());
					ImGui::Text("Milliseconds Passed: %.0f", services.ticker.total_milliseconds_passed.count());
					ImGui::Text("Ticks Per Frame: %.2f", services.ticker.ticks_per_frame);
					ImGui::Text("Frames Per Second: %.2f", services.ticker.fps);
					ImGui::Separator();
					ImGui::Text("Random");
					ImGui::Text("Ten percent chance: %.2f", static_cast<float>(rng_test.sample) / static_cast<float>(rng_test.total));
					ImGui::Separator();
					if (ImGui::SliderFloat("DeltaTime Scalar", &services.ticker.dt_scalar, 0.0f, 2.f, "%.3f")) { services.ticker.scale_dt(); };
					if (ImGui::Button("Reset")) { services.ticker.reset_dt(); }
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Input")) {
					ImGui::Text("Current Input Device: %s", "TODO"); // XXX services.controller_map.is_gamepad() ? "Gamepad" : "Keyboard
					ImGui::Text("Gamepad Status: %s", services.controller_map.gamepad_connected() ? "Connected" : "Disconnected");
					ImGui::Text("Gamepad Enabled? %s", services.controller_map.is_gamepad_input_enabled() ? "Yes" : "No");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tutorial")) {
					ImGui::Text("Jump..: %s", player.tutorial.flags.test(text::TutorialFlags::jump) ? "Yes" : "No");
					ImGui::Text("Shoot.: %s", player.tutorial.flags.test(text::TutorialFlags::shoot) ? "Yes" : "No");
					ImGui::Text("Sprint: %s", player.tutorial.flags.test(text::TutorialFlags::sprint) ? "Yes" : "No");
					ImGui::Text("Inv...: %s", player.tutorial.flags.test(text::TutorialFlags::inventory) ? "Yes" : "No");
					ImGui::Text("Map...: %s", player.tutorial.flags.test(text::TutorialFlags::map) ? "Yes" : "No");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Transponder")) {
					ImGui::Text("Voice Shipment..: %i", player.transponder.shipments.voice.get_residue());
					ImGui::Text("Emotion Shipment: %i", player.transponder.shipments.emotion.get_residue());
					ImGui::Text("Item Shipment...: %i", player.transponder.shipments.item.get_residue());
					ImGui::Text("Quest Shipment..: %i", player.transponder.shipments.quest.get_residue());
					ImGui::Separator();
					ImGui::Text("MiniMap Scale..: %f", game_state.get_current_state().inventory_window.minimap.get_scale());
					ImGui::Text("MiniMap Ratio..: %f", game_state.get_current_state().inventory_window.minimap.get_ratio());
					ImGui::Text("MiniMap X Pos..: %f", game_state.get_current_state().inventory_window.minimap.get_position().x);
					ImGui::Text("MiniMap Y Pos..: %f", game_state.get_current_state().inventory_window.minimap.get_position().y);
					ImGui::Separator();
					ImGui::Text("MiniMap Extent Left..: %f", game_state.get_current_state().inventory_window.minimap.get_extent().left);
					ImGui::Text("MiniMap Extent Top..: %f", game_state.get_current_state().inventory_window.minimap.get_extent().top);
					ImGui::Text("MiniMap Extent Width..: %f", game_state.get_current_state().inventory_window.minimap.get_extent().width);
					ImGui::Text("MiniMap Extent Height..: %f", game_state.get_current_state().inventory_window.minimap.get_extent().height);
					ImGui::Separator();
					ImGui::Text("MiniMap Center X Pos..: %f", game_state.get_current_state().inventory_window.minimap.get_center_position().x);
					ImGui::Text("MiniMap Center Y Pos..: %f", game_state.get_current_state().inventory_window.minimap.get_center_position().y);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("NPC")) {
					ImGui::Separator();
					for (auto& entry : services.data.npc_locations) {
						ImGui::Text("NPC: %i", entry.first);
						ImGui::SameLine();
						ImGui::Text(" ; Location: %i", entry.second);
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Sound")) {
					ImGui::Separator();
					ImGui::Text("Sound pool size: %i", static_cast<int>(services.soundboard.number_of_playng_sounds()));
					ImGui::Separator();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Music")) {
					ImGui::Separator();
					ImGui::Checkbox("Music Player", &playtest.m_musicplayer);
					ImGui::Text("Volume: ");
					ImGui::SameLine();
					ImGui::SliderFloat("##vol", &services.music.volume.multiplier, 0.f, 1.f, "%.3f");
					if (playtest.m_musicplayer && services.music.global_off()) {
						services.music.turn_on();
					} else if (!playtest.m_musicplayer && !services.music.global_off()) {
						services.music.turn_off();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Story")) {
					ImGui::Separator();
					ImGui::Text("Piggybacking? %s", static_cast<bool>(player.piggybacker) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Quest Progress:");
					ImGui::Text("Bit: %i", services.quest.get_progression(QuestType::npc, 20));
					ImGui::Text("Justin: %i", services.quest.get_progression(QuestType::npc, 24));
					ImGui::Text("Justin Hidden: %i", services.quest.get_progression(fornani::QuestType::hidden_npcs, 24));
					ImGui::Text("Gobe: %i", services.quest.get_progression(QuestType::npc, 3));
					ImGui::Text("Bryn's Notebook: %i", services.quest.get_progression(QuestType::inspectable, 1));
					ImGui::Text("Boiler: %i", services.quest.get_progression(QuestType::inspectable, 110));
					ImGui::Separator();
					ImGui::Text("Stats:");
					ImGui::Text("Death count: %i", services.stats.player.death_count.get_count());
					ImGui::Text("Bullets fired: %i", services.stats.player.bullets_fired.get_count());
					ImGui::Text("Guns collected: %i", services.stats.player.guns_collected.get_count());
					ImGui::Text("Items collected: %i", services.stats.player.items_collected.get_count());
					ImGui::Text("Orbs collected: %i", services.stats.treasure.total_orbs_collected.get_count());
					ImGui::Text("Blue orbs collected: %i", services.stats.treasure.blue_orbs.get_count());
					ImGui::Text("Highest indicator amount: %i", services.stats.treasure.highest_indicator_amount.get_count());
					ImGui::Text("Rooms discovered: %i", services.stats.world.rooms_discovered.get_count());
					ImGui::Text("Enemies killed: %i", services.stats.enemy.enemies_killed.get_count());
					ImGui::EndTabItem();
				}
				if (flags.test(GameFlags::in_game)) {

					if (ImGui::BeginTabItem("State")) {
						if (ImGui::Button("Main Menu")) {
							game_state.set_current_state(std::make_unique<automa::MainMenu>(services, player, "main"));
							flags.reset(GameFlags::in_game);
						}
						if (ImGui::Button("Caster")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 219);
							player.set_position({32 * 2, 32 * 27});
						}
						if (ImGui::Button("Minigus")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 115);
							player.set_position({32 * 3, 32 * 8});
						}
						if (ImGui::Button("Hangar")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 112);
							player.set_position({32 * 2, 32 * 8});
						}
						if (ImGui::Button("Canopy")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 224);
							player.set_position({32 * 4, 32 * 8});
						}
						if (ImGui::Button("Hideout")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 125);
							player.set_position({32 * 8, 32 * 2});
						}
						if (ImGui::Button("Shaft")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 107);
							player.set_position({32 * 6, 32 * 4});
						}
						if (ImGui::Button("Atrium 1")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 102);
							player.set_position({32 * 45, 32 * 56});
						}
						if (ImGui::Button("Corridor 2")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 104);
							player.set_position({7 * 32, 7 * 32});
						}
						if (ImGui::Button("Arena")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 121);
							player.set_position({3 * 32, 9 * 32});
						}
						if (ImGui::Button("Bunker")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 124);
							player.set_position({4 * 32, 3 * 32});
						}
						if (ImGui::Button("Cargo")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 103);
							player.set_position({7 * 32, 7 * 32});
						}
						if (ImGui::Button("Prison")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 100);
							player.set_position({7 * 32, 7 * 32});
						}
						if (ImGui::Button("Lab")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 110);
							player.set_position({7 * 32, 9 * 32});
						}
						ImGui::Text("Test Levels:");
						if (ImGui::Button("Junkyard")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 3001);
							player.set_position({4 * 32, 9 * 32});
						}
						if (ImGui::Button("Bridge")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 6001);
							player.set_position({4 * 32, 9 * 32});
						}
						if (ImGui::Button("Weather")) {
							services.soundboard.flags.menu.set(audio::Menu::select);;
							game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo"));
							game_state.get_current_state().init(services, 9901);
							player.set_position({7 * 32, 7 * 32});
						}
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Player")) {
						if (ImGui::BeginTabBar("PlayerTabBar", tab_bar_flags)) {
							if (ImGui::BeginTabItem("Animation")) {
								ImGui::Text("Animation: %s", player.animation.animation.label.data());
								ImGui::Separator();
								ImGui::Text("Current Frame: %i", player.animation.animation.frame.get_count());
								ImGui::Text("Loop: %i", player.animation.animation.loop.get_count());
								ImGui::Text("Frame Timer: %i", player.animation.animation.frame_timer.get_cooldown());
								ImGui::Text("Complete? %s", player.animation.animation.complete() ? "Yes" : "No");
								ImGui::Text("One Off? %s", player.animation.animation.params.num_loops > -1 ? "Yes" : "No");
								ImGui::Text("Repeat Last Frame? %s", player.animation.animation.params.repeat_last_frame ? "Yes" : "No");
								ImGui::Text("Idle Timer: %i", player.animation.idle_timer.get_count());
								ImGui::Separator();
								ImGui::Text("idle...: %s", player.animation.state == player::AnimState::idle ? "flag set" : "");
								ImGui::Text("run....: %s", player.animation.state == player::AnimState::run ? "flag set" : "");
								ImGui::Text("stop...: %s", player.animation.state == player::AnimState::stop ? "flag set" : "");
								ImGui::Text("turn...: %s", player.animation.state == player::AnimState::turn ? "flag set" : "");
								ImGui::Text("hurt...: %s", player.animation.state == player::AnimState::hurt ? "flag set" : "");
								ImGui::Text("shpturn: %s", player.animation.state == player::AnimState::sharp_turn ? "flag set" : "");
								ImGui::Text("rise...: %s", player.animation.state == player::AnimState::rise ? "flag set" : "");
								ImGui::Text("suspend: %s", player.animation.state == player::AnimState::suspend ? "flag set" : "");
								ImGui::Text("fall...: %s", player.animation.state == player::AnimState::fall ? "flag set" : "");
								ImGui::Separator();
								ImGui::Text("land...: %s", player.animation.state == player::AnimState::land ? "flag set" : "");
								ImGui::Text("roll...: %s", player.animation.state == player::AnimState::roll ? "flag set" : "");
								ImGui::Text("slide..: %s", player.animation.state == player::AnimState::slide ? "flag set" : "");
								ImGui::Text("getup..: %s", player.animation.state == player::AnimState::get_up ? "flag set" : "");
								ImGui::Separator();
								ImGui::Text("dash...: %s", player.animation.state == player::AnimState::dash ? "flag set" : "");
								ImGui::Text("sprint.: %s", player.animation.state == player::AnimState::sprint ? "flag set" : "");
								ImGui::Text("wlslide: %s", player.animation.state == player::AnimState::wallslide ? "flag set" : "");
								ImGui::Text("wljump.: %s", player.animation.state == player::AnimState::walljump ? "flag set" : "");
								ImGui::Text("inspect: %s", player.animation.state == player::AnimState::inspect ? "flag set" : "");
								ImGui::Text("die....: %s", player.animation.state == player::AnimState::die ? "flag set" : "");
								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Weapon")) {
								ImGui::Text("Current Weapon:");
								ImGui::Separator();
								if (player.hotbar) {
									auto& gun = player.equipped_weapon();
									ImGui::Text(gun.label.data());
									ImGui::Text("Ammo Capacity: %i", gun.ammo.get_capacity());
									ImGui::Text("Ammo Count: %i", gun.ammo.get_count());
								}
								ImGui::Separator();
								ImGui::Text("Loadout:");
								if (player.arsenal) {
									for (auto& gun : player.arsenal.value().get_loadout()) { ImGui::Text(gun.get()->label.data()); }
								}
								ImGui::Text("Hotbar:");
								if (player.hotbar) {
									for (auto& gun : player.hotbar.value().get_ids()) { ImGui::Text("%i", gun); }
								}
								ImGui::Text("Player has arsenal? %s", player.arsenal ? "Yes" : "No");
								ImGui::Text("Loadout Size: %i", player.arsenal ? player.arsenal.value().size() : 0);
								playtest_sync();
								ImGui::Checkbox("Bryn's Gun", &playtest.weapons.bryn);
								toggle_weapon(playtest.weapons.bryn, 0);
								ImGui::Checkbox("Plasmer", &playtest.weapons.plasmer);
								toggle_weapon(playtest.weapons.plasmer, 1);
								ImGui::Checkbox("Tomahawk", &playtest.weapons.tomahawk);
								toggle_weapon(playtest.weapons.tomahawk, 3);
								ImGui::Checkbox("Grappling Hook", &playtest.weapons.grapple);
								toggle_weapon(playtest.weapons.grapple, 4);
								ImGui::Checkbox("Grenade Launcher", &playtest.weapons.grenade);
								toggle_weapon(playtest.weapons.grenade, 5);
								ImGui::Checkbox("Staple Gun", &playtest.weapons.staple_gun);
								toggle_weapon(playtest.weapons.staple_gun, 8);
								ImGui::Checkbox("Indie", &playtest.weapons.indie);
								toggle_weapon(playtest.weapons.indie, 9);
								ImGui::Checkbox("Gnat", &playtest.weapons.gnat);
								toggle_weapon(playtest.weapons.gnat, 10);
								ImGui::Separator();

								if (ImGui::Button("Clear Loadout")) {
									if (player.arsenal) { player.arsenal = {}; }
									playtest.weapons = {};
								}
								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Catalog")) {
								ImGui::Separator();
								ImGui::Text("Inventory");
								for (auto& item : player.catalog.categories.inventory.items) {
									ImGui::Text(item.label.data());
									ImGui::SameLine();
									ImGui::Text(" : %i", item.get_quantity());
								}
								ImGui::Separator();

								ImGui::Text("Abilities");
								ImGui::Checkbox("Dash", &playtest.b_dash);
								ImGui::Checkbox("Shield", &playtest.b_shield);
								ImGui::Checkbox("Wallslide", &playtest.b_wallslide);
								ImGui::Checkbox("Double Jump", &playtest.b_doublejump);
								playtest.b_dash ? player.catalog.categories.abilities.give_ability(player::Abilities::dash) : player.catalog.categories.abilities.remove_ability(player::Abilities::dash);
								playtest.b_shield ? player.catalog.categories.abilities.give_ability(player::Abilities::shield) : player.catalog.categories.abilities.remove_ability(player::Abilities::shield);
								playtest.b_wallslide ? player.catalog.categories.abilities.give_ability(player::Abilities::wall_slide) : player.catalog.categories.abilities.remove_ability(player::Abilities::wall_slide);
								playtest.b_doublejump ? player.catalog.categories.abilities.give_ability(player::Abilities::double_jump) : player.catalog.categories.abilities.remove_ability(player::Abilities::double_jump);

								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Parameters")) {
								ImGui::Text("Vertical Movement");
								ImGui::SliderFloat("GRAVITY", &player.physics_stats.grav, -2.0f, 2.8f, "%.5f");
								ImGui::SliderFloat("JUMP VELOCITY", &player.physics_stats.jump_velocity, 0.5f, 18.0f, "%.5f");
								ImGui::SliderFloat("JUMP RELEASE MULTIPLIER", &player.physics_stats.jump_release_multiplier, 0.005f, 1.f, "%.5f");
								ImGui::SliderFloat("MAX Y VELOCITY", &player.physics_stats.maximum_velocity.y, 1.0f, 60.0f);

								ImGui::Separator();
								ImGui::Text("Horizontal Movement");
								ImGui::SliderFloat("AIR MULTIPLIER", &player.physics_stats.air_multiplier, 0.0f, 5.0f);
								ImGui::SliderFloat("GROUND FRICTION", &player.physics_stats.ground_fric, 0.800f, 1.000f);
								ImGui::SliderFloat("AIR FRICTION", &player.physics_stats.air_fric, 0.800f, 1.000f);
								ImGui::SliderFloat("GROUND SPEED", &player.physics_stats.x_acc, 0.0f, 3.f);
								ImGui::SliderFloat("SPRINT MULTIPLIER", &player.physics_stats.sprint_multiplier, 1.0f, 4.0f);
								ImGui::SliderFloat("MAX X VELOCITY", &player.physics_stats.maximum_velocity.x, 1.0f, 10.0f);

								ImGui::Separator();
								ImGui::Text("Abilities");
								ImGui::SliderFloat("Dash Speed", &player.physics_stats.dash_speed, 1.0f, 30.0f);
								ImGui::SliderFloat("Vertical Dash Multiplier", &player.physics_stats.vertical_dash_multiplier, 0.0f, 10.0f);
								ImGui::SliderFloat("Dash Dampen", &player.physics_stats.dash_dampen, 0.7f, 2.0f);
								ImGui::SliderFloat("Wallslide Speed", &player.physics_stats.wallslide_speed, 0.2f, 3.0f);

								ImGui::Separator();
								if (ImGui::Button("Save Parameters")) { services.data.save_player_params(player); }
								ImGui::EndTabItem();
							}
							ImGui::EndTabBar();
						}
						ImGui::EndTabItem();
					}
				}
				ImGui::EndTabBar();
			}
		}
		ImVec2 prev_size = ImGui::GetWindowSize();
		ImGui::End();
	}
}

void Game::take_screenshot(sf::Texture& screencap) {
	services.window->screencap.update(services.window->get());
	std::time_t const now = std::time(nullptr);

	std::time_t time = std::time({});
	char time_string[std::size("yyyy-mm-ddThh:mm:ssZ")];
	std::strftime(std::data(time_string), std::size(time_string), "%FT%TZ", std::gmtime(&time));
	std::string time_str = time_string;

	std::erase_if(time_str, [](auto const& c) { return c == ':' || isspace(c); });
	std::string filename = "screenshot_" + time_str + ".png";
	if (screencap.copyToImage().saveToFile(filename)) { std::cout << "screenshot saved to " << filename << std::endl; }
}

void Game::playtest_sync() {
	if (!player.arsenal) {
		playtest.weapons = {};
		return;
	}
	playtest.weapons.bryn = player.arsenal.value().has(0);
	playtest.weapons.plasmer = player.arsenal.value().has(1);
	playtest.weapons.tomahawk = player.arsenal.value().has(3);
	playtest.weapons.grapple = player.arsenal.value().has(4);
	playtest.weapons.grenade = player.arsenal.value().has(5);
	playtest.weapons.staple_gun = player.arsenal.value().has(8);
	playtest.weapons.indie = player.arsenal.value().has(9);
	playtest.weapons.gnat = player.arsenal.value().has(10);
}

void Game::toggle_weapon(bool flag, int id) {
	if (!player.arsenal && flag) {
		player.push_to_loadout(id);
		return;
	}
	if (player.arsenal) {
		if (flag && !player.arsenal.value().has(id)) {
			player.push_to_loadout(id);
		} else if (!flag && player.arsenal.value().has(id)) {
			player.pop_from_loadout(id);
		}
	}
}

} // namespace fornani