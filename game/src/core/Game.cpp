
#include "fornani/core/Game.hpp"
#include <steam/steam_api.h>
#include <ctime>
#include "fornani/automa/states/Dojo.hpp"
#include "fornani/automa/states/MainMenu.hpp"
#include "fornani/automa/states/SettingsMenu.hpp"
#include "fornani/automa/states/Trial.hpp"
#include "fornani/graphics/Colors.hpp"
#include "fornani/gui/ActionContextBar.hpp"
#include "fornani/setup/WindowManager.hpp"
#include "fornani/utils/Math.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani {

static double average_frame_time{};

Game::Game(char** argv, WindowManager& window, Version& version, capo::IEngine& audio_engine) : services(argv, version, window, audio_engine), player(services), game_state(services, player, automa::MenuType::main) {

	/* Set up ImGui Context */
	auto wContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(wContext);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	// NANI_LOG_INFO(m_logger, "ImGui IO.Fonts size: {}", io.Fonts->Fonts.size);
	if (!ImGui::SFML::Init(services.window->get())) {
		NANI_LOG_ERROR(m_logger, "ImGui-SFML failed to initialize the window.");
		shutdown();
		return;
	}

	// controls
	services.data.load_controls(services.controller_map);
	services.data.load_settings();

	m_background = std::make_unique<graphics::Background>(services, "black");
}

void Game::run(capo::IEngine& audio_engine, bool demo, int room_id, std::filesystem::path levelpath, sf::Vector2f player_position) {

	if (services.window->is_fullscreen()) { services.app_flags.set(automa::AppFlags::fullscreen); }
	services.set_editor(false);

	measurements.win_size.x = services.window->get().getSize().x;
	measurements.win_size.y = services.window->get().getSize().y;
	auto entire_window = sf::View(sf::FloatRect{{}, sf::Vector2f{sf::VideoMode::getDesktopMode().size}});

	if (demo) {
		services.debug_flags.set(automa::DebugFlags::demo_mode);
		flags.set(GameFlags::in_game);
		// services.music_player.turn_off();
		services.data.load_progress(player, services.editor_settings.save_file);
		game_state.set_current_state(std::make_unique<automa::Dojo>(services, player, "dojo", room_id, levelpath.filename().string()));
		services.state_controller.demo_level = room_id;
		NANI_LOG_INFO(m_logger, "Launching demo in room {} from folder {} ", room_id, levelpath.filename().string());
		services.state_controller.player_position = player_position;
		player.set_position(player_position);
		player.set_direction(Direction{LR::left});
	}

	gui::ActionContextBar ctx_bar(services);

	NANI_LOG_INFO(m_logger, "Success");
	services.stopwatch.stop();
	services.stopwatch.print_time("game started");

	while (services.window->get().isOpen()) {

		static bool zooming{};

		auto smp = random::percent_chance(10) ? 1 : 0;
		rng_test.sample += smp;
		++rng_test.total;

		if (services.state_controller.actions.test(automa::Actions::shutdown)) {
			NANI_LOG_INFO(m_logger, "Shutdown");
			break;
		}
		if (services.death_mode()) { flags.reset(GameFlags::in_game); }

		services.ticker.start_frame();

		services.controller_map.set_keyboard_input_detected(false);
		while (std::optional const event = services.window->get().pollEvent()) {
			if (event->is<sf::Event::Closed>()) {
				shutdown();
				return;
			}

			if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
				services.controller_map.set_keyboard_input_detected(true);
				services.controller_map.set_last_key_pressed(key_pressed->scancode);
				if (key_pressed->scancode == sf::Keyboard::Scancode::F12) { continue; }
				if (key_pressed->scancode == sf::Keyboard::Scancode::G && key_pressed->control) { services.toggle_greyblock_mode(); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::P && key_pressed->control) {
					services.toggle_debug();
					if (flags.test(GameFlags::playtest)) {
						flags.reset(GameFlags::playtest);
						services.soundboard.flags.menu.set(audio::Menu::forward_switch);
					} else {
						flags.set(GameFlags::playtest);
						services.soundboard.flags.menu.set(audio::Menu::backward_switch);
					}
				}
				if (key_pressed->scancode == sf::Keyboard::Scancode::R && key_pressed->control) { restart_trial(levelpath); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Equal) { take_screenshot(services.window->screencap); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Y) {
					auto view = services.window->get_view();
					view.zoom(0.5f);
					services.window->get().setView(view);
					zooming = !zooming;
				}
				if (key_pressed->scancode == sf::Keyboard::Scancode::Escape) { m_game_menu = {}; }
			}

			if (auto const* joystick_moved = event->getIf<sf::Event::JoystickMoved>()) {
				services.controller_map.set_keyboard_input_detected(false);
				auto jx = sf::Joystick::getAxisPosition(joystick_moved->joystickId, sf::Joystick::Axis::X);
				auto jy = sf::Joystick::getAxisPosition(joystick_moved->joystickId, sf::Joystick::Axis::Y);
				services.controller_map.set_joystick_throttle(sf::Vector2f{jx, jy});
			}

			services.controller_map.handle_event(*event);
			ImGui::SFML::ProcessEvent(services.window->get(), *event);
		}

		SteamAPI_RunCallbacks();

		bool has_focus = services.window->get().hasFocus();
		services.ticker.tick([this, has_focus, &ctx_bar = ctx_bar, &services = services, &audio_engine = audio_engine] {
			services.controller_map.update();
			services.music_player.update();
			if (services.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered && m_game_menu) {
				if (m_game_menu.value()->get_current_state().is_ready()) {
					m_game_menu = {};
					services.soundboard.flags.menu.set(audio::Menu::backward_switch);
				}
			}
			if (m_game_menu) {
				m_game_menu.value()->get_current_state().tick_update(services, audio_engine);
			} else {
				game_state.get_current_state().tick_update(services, audio_engine);
			}
			if (services.a11y.is_action_ctx_bar_enabled()) { ctx_bar.update(services); }
			if (game_state.get_current_state().flags.test(automa::GameStateFlags::settings_request)) {
				m_game_menu = std::make_unique<automa::StateManager>(services, player, automa::MenuType::settings);
				game_state.get_current_state().flags.reset(automa::GameStateFlags::settings_request);
			}
			if (game_state.get_current_state().flags.test(automa::GameStateFlags::controls_request)) {
				m_game_menu = std::make_unique<automa::StateManager>(services, player, automa::MenuType::controls);
				game_state.get_current_state().flags.reset(automa::GameStateFlags::controls_request);
			}
			if (game_state.get_current_state().get_type() == automa::StateType::menu) { m_background->update(services); }
		});
		if (m_game_menu) {
			m_game_menu.value()->get_current_state().frame_update(services);
		} else {
			game_state.get_current_state().frame_update(services);
		}

		if (services.state_controller.actions.consume(automa::Actions::screenshot)) { take_screenshot(services.window->screencap); }
		if (services.state_controller.actions.consume(automa::Actions::restart)) { restart_trial(levelpath); }

		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		ImGuiIO& io = ImGui::GetIO();
		io.IniFilename = NULL;
		io.LogFilename = NULL;
		io.MouseDrawCursor = flags.test(GameFlags::draw_cursor);
		services.window->get().setMouseCursorVisible(io.MouseDrawCursor);
		ImGui::SFML::Update(services.window->get(), m_frame_tracker.get_elapsed_time());
		m_frame_tracker.update();
		if (services.ticker.every_x_frames(60)) { average_frame_time = m_frame_tracker.get_average_frame_time(); }

#if not defined(FORNANI_PRODUCTION)
		if (flags.test(GameFlags::playtest)) { playtester_portal(services.window->get()); }
		flags.test(GameFlags::playtest) || demo ? flags.set(GameFlags::draw_cursor) : flags.reset(GameFlags::draw_cursor);
#endif

		services.window->get().clear();
		if (services.window->is_fullscreen()) { services.window->get().setView(entire_window); }
		if (game_state.get_current_state().get_type() == automa::StateType::menu) { m_background->render(services, services.window->get(), {}); }
		if (!zooming) { services.window->restore_view(); }

		if (m_game_menu) {
			m_game_menu.value()->get_current_state().render(services, services.window->get());
		} else {
			game_state.get_current_state().render(services, services.window->get());
		}

		if (services.a11y.is_action_ctx_bar_enabled()) { ctx_bar.render(services.window->get()); }

		ImGui::SFML::Render(services.window->get());
		services.window->get().display();

		services.ticker.end_frame();

		if (!m_game_menu) { game_state.process_state(services, player, *this); }
	}
	shutdown();
}

void Game::shutdown() { ImGui::SFML::Shutdown(); }

void Game::playtester_portal(sf::RenderWindow& window) {
	if (!flags.test(GameFlags::playtest)) { return; }

	auto const& map = game_state.get_current_state().get_map();

	bool* b_debug{};
	static bool limit_framerate{true};
	static int frame_limit{60};
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
					ImGui::Checkbox("Limit Framerate", &limit_framerate);
					ImGui::InputInt("Frame Limit", &frame_limit);
					limit_framerate ? services.window->get().setFramerateLimit(frame_limit) : services.window->get().setFramerateLimit(0);
					if (ImGui::Button("Exit to Main Menu")) { game_state.set_current_state(std::make_unique<automa::MainMenu>(services, player)); }
					ImGui::Text("In Game? %s", services.in_game() ? "Yes" : "No");
					ImGui::Text("debug mode: %s", services.debug_mode() ? "Enabled" : "Disabled");
					ImGui::Text("demo mode: %s", services.demo_mode() ? "Enabled" : "Disabled");
					if (ImGui::Button("Toggle Debug Mode")) { services.toggle_debug(); }
					if (ImGui::Button("Toggle Demo Mode")) { services.debug_flags.test(automa::DebugFlags::demo_mode) ? services.debug_flags.reset(automa::DebugFlags::demo_mode) : services.debug_flags.set(automa::DebugFlags::demo_mode); }
					if (ImGui::Button("Toggle Greyblock Mode")) {
						services.debug_flags.set(automa::DebugFlags::greyblock_trigger);
						services.debug_flags.test(automa::DebugFlags::greyblock_mode) ? services.debug_flags.reset(automa::DebugFlags::greyblock_mode) : services.debug_flags.set(automa::DebugFlags::greyblock_mode);
					}
					ImGui::Separator();
					ImGui::Text("Camera");
					if (ImGui::Button("Toggle Freedom")) { services.camera_controller.is_free() ? services.camera_controller.constrain() : services.camera_controller.free(); }
					ImGui::Separator();
					ImGui::Text("Ticker");
					ImGui::Text("dt: %.8f", services.ticker.dt.count());
					ImGui::Separator();
					ImGui::Text("Seconds Passed: %.2f", services.ticker.total_seconds_passed.count());
					ImGui::Text("Seconds Passed In-Game: %.2f", services.ticker.in_game_seconds_passed.count());
					ImGui::Text("Milliseconds Passed: %.0f", services.ticker.total_milliseconds_passed.count());
					ImGui::Text("Ticks Per Frame: %.2f", services.ticker.ticks_per_frame);
					ImGui::Text("Frames Per Second: %.2f", services.ticker.fps);
					ImGui::Text("Average Frame Time: %.4fms", average_frame_time);
					ImGui::Separator();
					if (ImGui::SliderFloat("DeltaTime Scalar", &services.ticker.dt_scalar, 0.0f, 2.f, "%.3f")) { services.ticker.scale_dt(); };
					if (ImGui::Button("Reset")) { services.ticker.reset_dt(); }
					ImGui::Separator();
					ImGui::Text("World Time: %s", services.world_clock.get_string().c_str());
					ImGui::Text("Time of Day: %s", services.world_clock.tod_as_string(services.world_clock.get_time_of_day()));
					ImGui::Text("Previous Time of Day: %s", services.world_clock.tod_as_string(services.world_clock.get_previous_time_of_day()));
					static int clock_speed{services.world_clock.get_rate()};
					if (ImGui::Button("Dawn")) { services.world_clock.set_time(5, 59); }
					if (ImGui::Button("Morning")) { services.world_clock.set_time(6, 59); }
					if (ImGui::Button("Dusk")) { services.world_clock.set_time(17, 59); }
					if (ImGui::Button("Night")) { services.world_clock.set_time(18, 59); }
					ImGui::Text("World clock transitioning? %s", services.world_clock.is_transitioning() ? "yes" : "no");
					ImGui::SliderInt("Clock Speed", &clock_speed, 4, 196);
					services.world_clock.set_speed(clock_speed);
					ImGui::Separator();
					ImGui::Text("Active Projectiles: %i", services.map_debug.active_projectiles);
					ImGui::Separator();

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("World")) {
					if (map) {
						ImGui::Separator();
						if (ImGui::Button("Reveal World in MiniMap")) {
							for (auto const& entry : services.data.map_table["rooms"].as_array()) { services.data.reveal_room(entry["room_id"].as<int>()); }
						}
						ImGui::Separator();
						map->get().debug();
						ImGui::Text("Transition State: %s", map->get().transition.as_string().c_str());
						ImGui::Text("Transition Cooldown: %.5f", map->get().transition.get_cooldown());
						ImGui::SeparatorText("Entities");
						ImGui::Text("Active Loot: %i", map->get().active_loot.size());
						ImGui::Indent();
						for (auto& loot : map->get().active_loot) { ImGui::Text("Loot Size: %i", loot.get_size()); }
						ImGui::Text("Active Projectiles: %i", map->get().active_projectiles.size());
						ImGui::Text("Active Emitters: %i", map->get().get_active_emitters_size());
						ImGui::Separator();
						ImGui::Text("Registered Colliders: %i", map->get().num_colliders());
						ImGui::Text("Registered Chunks: %i", map->get().num_registered_chunks());
						if (ImGui::TreeNode("Chunk Collisions")) {
							for (auto i = 0; i < map->get().num_registered_chunks(); ++i) { ImGui::Text("Colliders in chunk %i: %i", i, map->get().num_colliders_in_chunk(i)); }
							ImGui::TreePop();
						}
						ImGui::Text("Collision calculations: %i", map->get().num_collision_checks);

						ImGui::Separator();
						ImGui::Text("Calculated chunks for player:");
						if (player.has_collider()) {
							for (auto const& chunk : player.get_collider().print_chunks()) {
								ImGui::SameLine();
								ImGui::Text("[%s]", chunk);
							}
						}
					}
					ImGui::Separator();
					ImGui::Text("Save Point ID: %i", services.state_controller.save_point_id);
					ImGui::Separator();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Data")) {
					ImGui::Text("Destroyed Inspectables: ");
					for (auto& i : services.data.get_destroyed_inspectables()) { ImGui::Text("%i, ", i); }
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tests")) {
					if (ImGui::Button("Test Notification")) { services.notifications.push_notification(services, "This is a test notification."); }
					ImGui::Text("Angle");
					ImGui::Text("{-1.f, 1.f} ; down-left: %.2f", util::get_angle_from_direction({-1.f, 1.f}));
					ImGui::Text("{1.f, -1.f} ; top-right: %.2f", util::get_angle_from_direction({1.f, -1.f}));
					ImGui::Text("{1.f, 0.f} ; horiz-right: %.2f", util::get_angle_from_direction({1.f, 0.f}));
					ImGui::Separator();
					ImGui::Text("Parity");
					ImGui::Text("-1 and 2: %s", util::same_parity(-1.f, 2.f) ? "Yes" : "No");
					ImGui::Text("3 and 5: %s", util::same_parity(3.f, 5.f) ? "Yes" : "No");
					ImGui::Text("-0.001 and 5: %s", util::same_parity(-0.001f, 5.f) ? "Yes" : "No");
					ImGui::Text("-0.1 and -5: %s", util::same_parity(-0.1f, -5.f) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Random");
					ImGui::Text("Ten percent chance: %.2f", static_cast<float>(rng_test.sample) / static_cast<float>(rng_test.total));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Input")) {
					ImGui::Text("Current Input Device: %s", services.controller_map.last_controller_type_used() == config::ControllerType::gamepad ? "Gamepad" : "Keyboard");
					ImGui::Text("Gamepad Status: %s", services.controller_map.gamepad_connected() ? "Connected" : "Disconnected");
					ImGui::Text("Gamepad Enabled? %s", services.controller_map.is_gamepad_input_enabled() ? "Yes" : "No");
					ImGui::Text("Action Set: %i", services.controller_map.get_action_set());
					ImGui::Text("Joystick Throttle: %.3f", services.controller_map.get_joystick_throttle().x);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Time Trials")) {
					auto list = services.data.time_trial_registry.readout_attempts(9902);
					if (list) {
						for (auto const& time : list.value()) { ImGui::Text("%.3f", time.time); }
					}
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
				if (ImGui::BeginTabItem("Enemy")) {
					ImGui::Separator();
					for (auto& entry : services.data.fallen_enemies) {
						ImGui::Text("Room ID: %i", entry.code.first);
						ImGui::SameLine();
						ImGui::Text(" ; External ID: %i", entry.code.second);
						ImGui::SameLine();
						ImGui::Text(" ; Respawn Distance: %i", entry.respawn_distance);
						if (entry.permanent) {
							ImGui::SameLine();
							ImGui::Text(" ; permanent");
						}
					}
					ImGui::Separator();
					ImGui::Text("Distance Traveled: %i", player.visit_history.distance_traveled());
					ImGui::Text("Distance Traveled from 223: %i", player.visit_history.distance_traveled_from(223));
					ImGui::Text("Visit History: ");
					for (auto& room : player.visit_history.rooms_visited) {
						ImGui::Text("%i, ", room);
						ImGui::SameLine();
					}
					ImGui::Separator();
					ImGui::Text("Room Deque: ");
					for (auto& room : player.visit_history.room_deque) {
						ImGui::Text("%i, ", room);
						ImGui::SameLine();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Audio")) {
					ImGui::Separator();
					static float sfxvol{services.soundboard.get_volume()};
					ImGui::SliderFloat("##sfxvol", &sfxvol, 0.f, 1.f);
					services.soundboard.set_volume(sfxvol);
					ImGui::Text("Soundboard Volume %f", services.soundboard.get_volume());
					ImGui::Text("Sound pool size: %i", static_cast<int>(services.soundboard.number_of_playng_sounds()));
					ImGui::Separator();
					if (ImGui::BeginTabBar("NPCs", tab_bar_flags)) {
						if (ImGui::BeginTabItem("Bryn")) {
							for (auto i = 0; i <= static_cast<int>(audio::NPCBryn::oeugh); ++i) {
								ImGui::PushID(i);
								if (ImGui::Button("OOO##i")) { services.soundboard.npc_map.at("bryn")(i); }
								ImGui::PopID();
							}
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Minigus")) {
							for (auto i = 0; i <= static_cast<int>(audio::NPCMinigus::grunt); ++i) {
								ImGui::PushID(i);
								if (ImGui::Button("OOO##i")) { services.soundboard.npc_map.at("minigus")(i); }
								ImGui::PopID();
							}
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Gobe")) {
							for (auto i = 0; i <= static_cast<int>(audio::NPCGobe::orewa); ++i) {
								ImGui::PushID(i);
								if (ImGui::Button("OOO##i")) { services.soundboard.npc_map.at("gobe")(i); }
								ImGui::PopID();
							}
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Mirin")) {
							for (auto i = 0; i <= static_cast<int>(audio::NPCMirin::haha); ++i) {
								ImGui::PushID(i);
								if (ImGui::Button("OOO##i")) { services.soundboard.npc_map.at("mirin")(i); }
								ImGui::PopID();
							}
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Story")) {
					ImGui::SeparatorText("Quest Info");
					for (auto i = 0; i < services.quest_registry.get_size(); ++i) { ImGui::Text("Title: %s", services.quest_registry.get_quest_metadata(i).get_title().data()); }

					for (auto i = 0; i < services.quest_registry.get_size(); ++i) {
						ImGui::Separator();
						auto const& meta = services.quest_registry.get_quest_metadata(i);
						auto tag = meta.get_tag().data();
						ImGui::Text(services.quest_table.print_progressions(tag).c_str());
					}
					ImGui::Separator();
					ImGui::Text("Piggybacking? %s", static_cast<bool>(player.piggybacker) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Quest Progress:");
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
				if (services.in_game()) {
					static float player_hp{};
					if (ImGui::BeginTabItem("Player")) {
						player.health.debug();
						if (ImGui::BeginTabBar("PlayerTabBar", tab_bar_flags)) {
							if (ImGui::BeginTabItem("General")) {
								ImGui::Text("Player");
								ImGui::Separator();
								ImGui::Text("Last Requested Direction: %s", player.controller.last_requested_direction().print().c_str());
								ImGui::Text("Desired Direction: %s", player.get_desired_direction().print().c_str());
								ImGui::Text("Actual Direction: %s", player.get_actual_direction().print().c_str());
								ImGui::Text("Collider Direction: %s", player.get_collider().get_direction().print().c_str());
								ImGui::Text("Ability Direction: %s", player.controller.get_ability_direction().print().c_str());
								ImGui::Separator();
								ImGui::Text("Grounded? %s", player.grounded() ? "Yes" : "No");
								ImGui::Text("World Grounded? %s", player.get_collider().perma_grounded() ? "Yes" : "No");
								ImGui::Text("Horizontal Movement: %f", player.controller.horizontal_movement());
								ImGui::Text("Push Time: %i", player.cooldowns.push.get());
								ImGui::Text("Acceleration Multiplier: %f", player.get_collider().acceleration_multiplier);
								ImGui::Separator();
								ImGui::SliderFloat("Antenna Force", &player.physics_stats.antenna_force, 0.1f, 3.f);
								ImGui::SliderFloat("Antenna Friction", &player.physics_stats.antenna_friction, 0.8f, 1.f);
								ImGui::Separator();
								ImGui::Text("Ability");
								ImGui::Text("Current: ");
								auto ability_type = player.controller.get_ability_type();
								if (ability_type) {
									ImGui::SameLine();
									switch (ability_type.value()) {
									case player::AbilityType::dash: ImGui::Text("dash"); break;
									case player::AbilityType::jump: ImGui::Text("jump"); break;
									case player::AbilityType::doublejump: ImGui::Text("doublejump"); break;
									case player::AbilityType::slide: ImGui::Text("slide"); break;
									case player::AbilityType::roll: ImGui::Text("roll"); break;
									case player::AbilityType::walljump: ImGui::Text("walljump"); break;
									case player::AbilityType::wallslide: ImGui::Text("wallslide"); break;
									case player::AbilityType::dash_kick: ImGui::Text("dash_kick"); break;
									}
								}
								ImGui::Separator();
								ImGui::Text("Trial Mode? %s", player.has_flag_set(player::PlayerFlags::trial) ? "Yes" : "No");
								ImGui::Text("Cutscene? %s", player.has_flag_set(player::PlayerFlags::cutscene) ? "Yes" : "No");
								ImGui::Text("Can Move? %s", player.controller.can_move() ? "Yes" : "No");
								ImGui::Text("Walking Autonomously? %s", player.controller.walking_autonomously() ? "Yes" : "No");
								ImGui::Separator();
								ImGui::Text("Sliding? %s", player.controller.is_sliding() ? "Yes" : "No");
								ImGui::Text("Can Slide? %s", player.can_slide() ? "Yes" : "No");
								ImGui::Text("Post-Slide Cooldown: %i", player.controller.post_slide.get());
								ImGui::Separator();
								ImGui::Text("Crouching? %s", player.controller.is_crouching() ? "Yes" : "No");
								ImGui::Text("Inspecting? %s", player.controller.inspecting() ? "Yes" : "No");
								ImGui::Separator();
								ImGui::Text("Jumping? %s", player.get_collider().jumping() ? "Yes" : "No");
								ImGui::Text("Can Jump? %s", player.can_jump() ? "Yes" : "No");
								ImGui::Text("Downhill? %s", player.get_collider().downhill() ? "Yes" : "No");
								ImGui::Text("Wallsliding? %s", player.controller.is_wallsliding() ? "Yes" : "No");
								ImGui::Text("Walljumping? %s", player.controller.is_walljumping() ? "Yes" : "No");
								ImGui::Text("On Ramp? %s", player.get_collider().on_ramp() ? "Yes" : "No");
								ImGui::Text("Submerged? %s", player.get_collider().has_flag_set(shape::ColliderFlags::submerged) ? "Yes" : "No");
								ImGui::Text("In Water? %s", player.get_collider().has_flag_set(shape::ColliderFlags::in_water) ? "Yes" : "No");
								ImGui::Separator();
								ImGui::Text("X Position: %.2f", player.get_collider().physics.position.x / constants::f_cell_size);
								ImGui::Text("Y Position: %.2f", player.get_collider().physics.position.y / constants::f_cell_size);
								ImGui::Text("X Velocity: %.2f", player.get_collider().physics.velocity.x);
								ImGui::Text("Y Velocity: %.2f", player.get_collider().physics.velocity.y);
								ImGui::Text("Apparent X Velocity: %.2f", player.get_collider().physics.apparent_velocity().x);
								ImGui::Text("Apparent Y Velocity: %.2f", player.get_collider().physics.apparent_velocity().y);
								ImGui::Text("Actual X Velocity: %.2f", player.get_collider().physics.actual_velocity().x);
								ImGui::Text("Actual Y Velocity: %.2f", player.get_collider().physics.actual_velocity().y);
								ImGui::Text("Actual Speed: %.2f", player.get_collider().physics.actual_speed());
								ImGui::Separator();
								ImGui::Text("Inventory Size: %i", static_cast<int>(player.catalog.inventory.items_view().size()));
								ImGui::Text("Visit History: ");
								for (auto& room : player.visit_history.rooms_visited) {
									ImGui::Text("%i, ", room);
									ImGui::SameLine();
								}
								ImGui::EndTabItem();
							}

							if (ImGui::BeginTabItem("Inventory")) {
								ImGui::Text("Equipped Items:");
								for (auto const& ei : player.catalog.inventory.equipped_items_view()) {
									ImGui::Text("[%i]", ei);
									ImGui::SameLine();
								}
								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Animation")) {
								ImGui::Text("Under construction...");
								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Weapon")) {
								ImGui::Text("Current Weapon:");
								ImGui::Separator();
								if (player.hotbar) {
									auto& gun = player.equipped_weapon();
									ImGui::Text("%s", gun.get_label().data());
									ImGui::Text("Ammo Capacity: %i", gun.ammo.get_capacity());
									ImGui::Text("Ammo Count: %i", gun.ammo.get_count());
								}
								ImGui::Separator();
								ImGui::Text("Loadout:");
								if (player.arsenal) {
									for (auto& gun : player.arsenal.value().get_loadout()) { ImGui::Text("%s", gun.get()->get_label().data()); }
								}
								ImGui::Text("Hotbar:");
								if (player.hotbar) {
									for (auto& gun : player.hotbar.value().get_tags()) { ImGui::Text("%s", gun.c_str()); }
								}
								ImGui::Text("Player has arsenal? %s", player.arsenal ? "Yes" : "No");
								ImGui::Text("Loadout Size: %zu", player.arsenal ? player.arsenal.value().size() : 0);

								if (ImGui::Button("Clear Loadout")) {
									if (player.arsenal) { player.arsenal = {}; }
								}
								ImGui::EndTabItem();
							}
							if (ImGui::BeginTabItem("Catalog")) {
								ImGui::Separator();
								ImGui::Text("Inventory");
								for (auto& item : player.catalog.inventory.items_view()) {
									ImGui::Text("%s", item->get_label().data());
									ImGui::SameLine();
								}
								ImGui::Separator();

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

	std::time_t const time = std::time({});
	char time_string[std::size("yyyy-mm-ddThh:mm:ssZ")];
	std::strftime(std::data(time_string), std::size(time_string), "%FT%TZ", std::gmtime(&time));
	auto time_str = std::string{time_string};

	std::erase_if(time_str, [](auto const& c) { return c == ':' || isspace(c); });
	auto destination = std::filesystem::path{services.finder.paths.screenshots.string()};
	auto filename = std::filesystem::path{"screenshot_" + time_str + ".png"};
	auto target = destination / filename;
	if (screencap.copyToImage().saveToFile(target.string())) { NANI_LOG_INFO(m_logger, "screenshot {} saved to {}", filename.string(), destination.string()); }
}

void Game::restart_trial(std::filesystem::path const& levelpath) { game_state.set_current_state(std::make_unique<automa::Trial>(services, player, "trial", services.state_controller.next_state, levelpath.filename().string())); }

} // namespace fornani
