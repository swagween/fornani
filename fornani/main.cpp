
// main

#include <SFML/Graphics.hpp>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "src/automa/StateManager.hpp"
#include "src/setup/EnumLookups.hpp"
#include "src/setup/ServiceLocator.hpp"
#include "src/utils/Grid.hpp"
#include "src/utils/Shape.hpp"

#include <imgui.h>
#include <random>
#include <imgui-SFML.h>

namespace {

auto SM = automa::StateManager{};
auto window = sf::RenderWindow();
auto minimap = sf::View();

sf::Texture screencap{};

int const NUM_TIMESTEPS = 64;
int TIME_STEP_MILLI = 0;
int frame{};
using Clock = std::chrono::steady_clock;
using Time = std::chrono::duration<float>;
auto elapsed_time = Time{};
auto elapsed_marker = Time{};
float time_markers[NUM_TIMESTEPS]{};
auto time_step = Time{std::chrono::milliseconds(TIME_STEP_MILLI)}; // FPS
float seconds = 0.0;
int FPS_counter = 0;
float FPS = 0.0;

int shake_counter = 0;
int frame_draw_counter{0};

sf::Vector2<uint32_t> win_size{};
float height_ratio{};
float width_ratio{};

void save_screenshot() {

	std::time_t const now = std::time(nullptr);

	std::string filedate = std::asctime(std::localtime(&now));
	std::erase_if(filedate, [](auto const& c) { return c == ':' || isspace(c); });
	std::string filename = "screenshot_" + filedate + ".png";

	if (screencap.copyToImage().saveToFile(filename)) { std::cout << "screenshot saved to " << filename << std::endl; }
}

static void show_overlay() {
	bool* debug{};
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
		if (ImGui::Begin("Debug Mode", debug, window_flags)) {
			ImGui::Text("Debug Window\n"
						"For Nani (beta version 1.0.0)");
			ImGui::Text("Window Focused: ");
			ImGui::SameLine();
			if (window.hasFocus()) {
				ImGui::Text("Yes");
			} else {
				ImGui::Text("No");
			}
			if (!window.hasFocus()) { window.RenderTarget::setActive(); }
			ImGui::Separator();
			ImGui::Text("Screen Dimensions X: %u", cam::screen_dimensions.x);
			ImGui::Text("Screen Dimensions Y: %u", cam::screen_dimensions.y);
			if (ImGui::IsMousePosValid()) {
				ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
			} else {
				ImGui::Text("Mouse Position: <invalid>");
			}
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
				if (ImGui::BeginTabItem("Time")) {
					ImGui::Separator();
					ImGui::Text("Ticker");
					ImGui::Text("dt: %.8f", svc::tickerLocator.get().dt.count());
					ImGui::Text("New Time: %.4f", svc::tickerLocator.get().new_time);
					ImGui::Text("Current Time: %.4f", svc::tickerLocator.get().current_time);
					ImGui::Text("Accumulator: %.4f", svc::tickerLocator.get().accumulator.count());
					ImGui::Separator();
					ImGui::Text("Seconds Passed: %.2f", svc::tickerLocator.get().total_seconds_passed.count());
					ImGui::Text("Ticks Per Frame: %.2f", svc::tickerLocator.get().ticks_per_frame);
					ImGui::Text("Frames Per Second: %.2f", svc::tickerLocator.get().fps);
					ImGui::Separator();
					ImGui::SliderFloat("Tick Rate (ms): ", &svc::tickerLocator.get().tick_rate, 0.0001f, 0.02f, "%.5f");
					ImGui::SliderFloat("Tick Multiplier: ", &svc::tickerLocator.get().tick_multiplier, 0.f, 64.f, "%.2f");
					if (ImGui::Button("Reset")) {
						svc::tickerLocator.get().tick_rate = 0.016f;
						svc::tickerLocator.get().tick_multiplier = 16;
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Console")) {
					ImGui::Separator();

					ImGui::Text("Console Active : %s", svc::consoleLocator.get().flags.test(gui::ConsoleFlags::active) ? "Yes" : "No");
					ImGui::Text("Console Writer Active : %s", svc::consoleLocator.get().writer.active() ? "Yes" : "No");
					ImGui::Text("Console Writer Complete : %s", svc::consoleLocator.get().writer.complete() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Player Transponder Skipping : %s", svc::playerLocator.get().transponder.skipped_ahead() ? "Yes" : "No");
					ImGui::Text("Player Transponder Exited : %s", svc::playerLocator.get().transponder.exited() ? "Yes" : "No");
					ImGui::Text("Player Transponder Requested Next : %s", svc::playerLocator.get().transponder.requested_next() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Player Restricted? : %s", svc::playerLocator.get().controller.restricted() ? "Yes" : "No");
					ImGui::Text("Player Inspecting? : %s", svc::playerLocator.get().controller.inspecting() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::SliderInt("Text Size", &svc::consoleLocator.get().writer.text_size, 6, 64);


					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Key States")) {
					ImGui::Text("Shift held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Shift triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Shift released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Left held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Left triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Left released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Right held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Right triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Right released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Up held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Up triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Up released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Down held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Down triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Down released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Z held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Z triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Z released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("X held: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("X triggered: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("X released: %s", svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Audio")) {
					ImGui::Separator();
					ImGui::Text("Music Volume");
					ImGui::SliderInt("##musvol", &svc::assetLocator.get().music_vol, 0, 100);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Player")) {
					if (ImGui::BeginTabBar("PlayerTabBar", tab_bar_flags)) {
						if (ImGui::BeginTabItem("Physics and Collision")) {
							ImGui::Text("Player Pos: (%.4f,%.4f)", svc::playerLocator.get().collider.physics.position.x, svc::playerLocator.get().collider.physics.position.y);
							ImGui::Text("Player Vel: (%.4f,%.4f)", svc::playerLocator.get().collider.physics.velocity.x, svc::playerLocator.get().collider.physics.velocity.y);
							ImGui::Text("Player Acc: (%.4f,%.4f)", svc::playerLocator.get().collider.physics.acceleration.x, svc::playerLocator.get().collider.physics.acceleration.y);
							ImGui::Text("Player Jer: (%.4f,%.4f)", svc::playerLocator.get().collider.physics.jerk.x, svc::playerLocator.get().collider.physics.jerk.y);
							ImGui::Separator();
							ImGui::Text("Player Grounded: %s", svc::playerLocator.get().grounded() ? "Yes" : "No");
							ImGui::Separator();
							ImGui::Text("Right Collision	: %s", svc::playerLocator.get().collider.collision_flags.test(shape::Collision::has_right_collision) ? "Yes" : "No");
							ImGui::Text("Left Collision		: %s", svc::playerLocator.get().collider.collision_flags.test(shape::Collision::has_left_collision) ? "Yes" : "No");
							ImGui::Text("Top Collision		: %s", svc::playerLocator.get().collider.collision_flags.test(shape::Collision::has_top_collision) ? "Yes" : "No");
							ImGui::Text("Bottom Collision	: %s", svc::playerLocator.get().collider.collision_flags.test(shape::Collision::has_bottom_collision) ? "Yes" : "No");
							
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Movement")) {
							ImGui::Text("Direction LR	: %s", svc::playerLocator.get().controller.direction.print_lr().c_str());
							ImGui::Text("Direction UND	: %s", svc::playerLocator.get().controller.direction.print_und().c_str());
							ImGui::Separator();
							ImGui::Text("Controller");
							ImGui::Text("Move Left : %s", svc::playerLocator.get().controller.get_controller_state(controllers::ControllerInput::move_x) < 0.f ? "Yes" : "No");
							ImGui::Text("Move Right : %s", svc::playerLocator.get().controller.get_controller_state(controllers::ControllerInput::move_x) > 0.f ? "Yes" : "No");
							ImGui::Text("Inspecting : %s", svc::playerLocator.get().controller.get_controller_state(controllers::ControllerInput::inspect) > 0.f ? "Yes" : "No");
							ImGui::Text("Facing : %s", svc::playerLocator.get().print_direction(true).c_str());
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Jump")) {
							ImGui::Text("Jump Request : %i", svc::playerLocator.get().controller.get_jump_request());
							ImGui::Text("Jump Released : %s", svc::playerLocator.get().controller.jump_released() ? "Yes" : "No");
							ImGui::Text("Can Jump : %s", svc::playerLocator.get().controller.can_jump() ? "Yes" : "No");
							ImGui::Text("Jumping? : %s", svc::playerLocator.get().controller.jumping() ? "Yes" : "No");
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Dash")) {
							ImGui::Text("Dash Value : %f", svc::playerLocator.get().controller.dash_value());
							ImGui::Text("Dash Request : %i", svc::playerLocator.get().controller.get_dash_request());
							ImGui::Text("Dash Count : %i", svc::playerLocator.get().controller.get_dash_count());
							ImGui::Text("Can Dash? : %s", svc::playerLocator.get().controller.can_dash() ? "Yes" : "No");
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Animation")) {
							ImGui::Text("Animation: %s", svc::playerLocator.get().animation.animation.label.c_str());
							ImGui::Separator();
							ImGui::Text("Current Frame: %i", svc::playerLocator.get().animation.animation.current_frame);
							ImGui::Text("Complete? %s", svc::playerLocator.get().animation.animation.complete() ? "Yes" : "No");
							ImGui::Text("One Off? %s", svc::playerLocator.get().animation.animation.params.num_loops > -1 ? "Yes" : "No");
							ImGui::Text("Repeat Last Frame? %s", svc::playerLocator.get().animation.animation.params.repeat_last_frame ? "Yes" : "No");
							ImGui::Separator();
							ImGui::Text("idle...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::idle) ? "flag set" : "");
							ImGui::Text("run....: %s", svc::playerLocator.get().animation.state.test(player::AnimState::run) ? "flag set" : "");
							ImGui::Text("stop...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::stop) ? "flag set" : "");
							ImGui::Text("turn...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::turn) ? "flag set" : "");
							ImGui::Text("jsquat.: %s", svc::playerLocator.get().animation.state.test(player::AnimState::jumpsquat) ? "flag set" : "");
							ImGui::Text("rise...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::rise) ? "flag set" : "");
							ImGui::Text("suspend: %s", svc::playerLocator.get().animation.state.test(player::AnimState::suspend) ? "flag set" : "");
							ImGui::Text("fall...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::fall) ? "flag set" : "");
							ImGui::Text("land...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::land) ? "flag set" : "");
							ImGui::Text("dash...: %s", svc::playerLocator.get().animation.state.test(player::AnimState::dash) ? "flag set" : "");
							ImGui::Text("inspect: %s", svc::playerLocator.get().animation.state.test(player::AnimState::inspect) ? "flag set" : "");
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Stats")) {
							ImGui::Text("Player Stats");
							ImGui::SliderInt("Max HP", &svc::playerLocator.get().player_stats.max_health, 3, 12);
							ImGui::SliderInt("HP", &svc::playerLocator.get().player_stats.health, 3, 12);
							ImGui::SliderInt("Max Orbs", &svc::playerLocator.get().player_stats.max_orbs, 99, 99999);
							ImGui::SliderInt("Orbs", &svc::playerLocator.get().player_stats.orbs, 0, 99999);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Parameter Tweaking")) {
							ImGui::Text("Vertical Movement");
							ImGui::SliderFloat("GRAVITY", &svc::playerLocator.get().physics_stats.grav, 0.f, 0.5f, "%.5f");
							ImGui::SliderFloat("JUMP VELOCITY", &svc::playerLocator.get().physics_stats.jump_velocity, 0.5f, 7.5f, "%.5f");
							ImGui::SliderFloat("JUMP RELEASE MULTIPLIER", &svc::playerLocator.get().physics_stats.jump_release_multiplier, 0.005f, 1.f, "%.5f");
							ImGui::SliderFloat("MAX Y VELOCITY", &svc::playerLocator.get().physics_stats.maximum_velocity.y, 1.0f, 60.0f);

							ImGui::Separator();
							ImGui::Text("Horizontal Movement");
							ImGui::SliderFloat("AIR MULTIPLIER", &svc::playerLocator.get().physics_stats.air_multiplier, 0.0f, 5.0f);
							ImGui::SliderFloat("GROUND FRICTION", &svc::playerLocator.get().physics_stats.ground_fric, 0.800f, 1.000f);
							ImGui::SliderFloat("AIR FRICTION", &svc::playerLocator.get().physics_stats.air_fric, 0.800f, 1.000f);
							ImGui::SliderFloat("GROUND SPEED", &svc::playerLocator.get().physics_stats.x_acc, 0.0f, 3.f);
							ImGui::SliderFloat("MAX X VELOCITY", &svc::playerLocator.get().physics_stats.maximum_velocity.x, 1.0f, 10.0f);

							ImGui::Separator();
							ImGui::Text("Dash");
							ImGui::SliderFloat("Dash Speed", &svc::playerLocator.get().physics_stats.dash_speed, 1.0f, 10.0f);
							ImGui::SliderFloat("Dash Multiplier", &svc::playerLocator.get().physics_stats.dash_multiplier, 0.0f, 10.0f);
							ImGui::SliderFloat("Dash Dampen", &svc::playerLocator.get().physics_stats.dash_dampen, 0.7f, 1.0f);

							ImGui::Separator();
							if (ImGui::Button("Save Parameters")) { svc::dataLocator.get().save_player_params(); }
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Misc")) {

							ImGui::Text("Alive? %s", svc::playerLocator.get().flags.state.test(player::State::alive) ? "Yes" : "No");

							ImGui::Text("Invincibility Counter: %i", svc::playerLocator.get().counters.invincibility);
							ImGui::Text("Spike Trigger: %s", svc::playerLocator.get().collider.spike_trigger ? "True" : "False");
							ImGui::Text("On Ramp: %s", svc::playerLocator.get().collider.on_ramp() ? "True" : "False");

							ImGui::Text("Grounded: %s", svc::playerLocator.get().grounded() ? "Yes" : "No");
							ImGui::EndTabItem();
						}

						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Weapon")) {
					if (ImGui::Button("Toggle Weapons")) {
						if (svc::playerLocator.get().arsenal.loadout.empty()) {
							//svc::playerLocator.get().weapons_hotbar = {arms::WEAPON_TYPE::BRYNS_GUN, arms::WEAPON_TYPE::PLASMER, arms::WEAPON_TYPE::CLOVER, arms::WEAPON_TYPE::NOVA};
							//svc::playerLocator.get().loadout.equipped_weapon = svc::playerLocator.get().weapons_hotbar.at(0);
							svc::playerLocator.get().arsenal.push_to_loadout(0);
							svc::playerLocator.get().arsenal.push_to_loadout(1);
						} else {
							//svc::playerLocator.get().weapons_hotbar.clear();
							svc::playerLocator.get().arsenal.loadout = {};
						}
					}

					ImGui::Text("Firing Direction LR: %s", svc::playerLocator.get().equipped_weapon().firing_direction.print_lr().c_str());
					ImGui::Text("Firing Direction UND : %s", svc::playerLocator.get().equipped_weapon().firing_direction.print_und().c_str());

					ImGui::Text("Cooling Down? %s", svc::playerLocator.get().equipped_weapon().cooling_down() ? "Yes" : "No");
					ImGui::Text("Cooldown Time %i", svc::playerLocator.get().equipped_weapon().cooldown_counter);

					ImGui::Separator();
					ImGui::Text("Equipped Weapon: %s", svc::playerLocator.get().equipped_weapon().label.c_str());
					ImGui::Text("UI color: %i", (int)svc::playerLocator.get().equipped_weapon().attributes.ui_color);
					ImGui::Text("Sprite Dimensions X: %.2f", (int)svc::playerLocator.get().equipped_weapon().sprite_dimensions.x);
					ImGui::Text("Sprite Dimensions Y: %.2f", (int)svc::playerLocator.get().equipped_weapon().sprite_dimensions.y);
					ImGui::Text("Barrel Point X: %f", (int)svc::playerLocator.get().equipped_weapon().barrel_point.x);
					ImGui::Text("Barrel Point Y: %f", (int)svc::playerLocator.get().equipped_weapon().barrel_point.y);
					ImGui::Separator();
					ImGui::Text("Weapon Stats: ");
					ImGui::Indent();
					ImGui::Text("Rate: (%i)", svc::playerLocator.get().equipped_weapon().attributes.rate);
					ImGui::Text("Cooldown: (%i)", svc::playerLocator.get().equipped_weapon().attributes.cooldown_time);
					ImGui::Text("Recoil: (%.2f)", svc::playerLocator.get().equipped_weapon().attributes.recoil);
					ImGui::Text("Spray Force: (%.2f)", svc::playerLocator.get().equipped_weapon().spray_behavior.expulsion_force);
					ImGui::Text("Spray Grav: (%.2f)", svc::playerLocator.get().equipped_weapon().spray_behavior.grav);

					ImGui::Separator();
					ImGui::Unindent();
					ImGui::Text("Projectile Stats: ");
					ImGui::Indent();
					ImGui::Text("Damage: (%i)", svc::playerLocator.get().equipped_weapon().projectile.stats.base_damage);
					ImGui::Text("Range: (%i)", svc::playerLocator.get().equipped_weapon().projectile.stats.range);
					ImGui::Text("Speed: (%.2f)", svc::playerLocator.get().equipped_weapon().projectile.stats.speed);
					ImGui::Text("Velocity: (%.4f,%.4f)", svc::playerLocator.get().equipped_weapon().projectile.physics.velocity.x, svc::playerLocator.get().equipped_weapon().projectile.physics.velocity.y);
					ImGui::Text("Position: (%.4f,%.4f)", svc::playerLocator.get().equipped_weapon().projectile.physics.position.x, svc::playerLocator.get().equipped_weapon().projectile.physics.position.y);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("General")) {
					ImGui::Text("Float Readout: (%.8f)", svc::floatReadoutLocator.get());
					ImGui::Text("Camera Position: (%.8f,%.8f)", svc::cameraLocator.get().position.x, svc::cameraLocator.get().position.y);
					ImGui::SliderFloat("Camera X Friction", &svc::cameraLocator.get().physics.ground_friction.x, 0.8f, 1.f, "%.5f");
					ImGui::SliderFloat("Camera Y Friction", &svc::cameraLocator.get().physics.ground_friction.y, 0.8f, 1.f, "%.5f");
					ImGui::SliderFloat("Camera Grav Force", &svc::cameraLocator.get().grav_force, 0.003f, 0.03f, "%.5f");
					ImGui::Text("Observed Camera Velocity: (%.8f,%.8f)", svc::cameraLocator.get().observed_velocity.x, svc::cameraLocator.get().observed_velocity.y);
					
					if (ImGui::Button("Save Screenshot")) { save_screenshot(); }
					ImGui::Separator();
					if (ImGui::Button("Toggle Greyblock Mode")) {
						if (svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state)) {
							svc::globalBitFlagsLocator.get().reset(svc::global_flags::greyblock_state);
							svc::globalBitFlagsLocator.get().set(svc::global_flags::greyblock_trigger);
						} else {
							svc::globalBitFlagsLocator.get().set(svc::global_flags::greyblock_state);
							svc::globalBitFlagsLocator.get().set(svc::global_flags::greyblock_trigger);
						}
					}
					ImGui::Text("Greybox Mode : %s", svc::globalBitFlagsLocator.get().test(svc::global_flags::greyblock_state) ? "On" : "Off");
					ImGui::Separator();
					ImGui::Text("Draw Calls: %u", frame_draw_counter);
					frame_draw_counter = 0;

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Resources")) {
					ImGui::Text("Size of Asset Manager (Bytes): %lu", sizeof(svc::assetLocator.get()));
					ImGui::Text("Size of Camera (Bytes): %lu", sizeof(svc::cameraLocator.get()));
					ImGui::Text("Size of Player (Bytes): %lu", sizeof(svc::playerLocator.get()));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("State")) {
					ImGui::Separator();
					ImGui::Text("State");
					ImGui::Text("Current State: ");
					ImGui::SameLine();
					ImGui::TextUnformatted(SM.get_current_state_string().c_str());
					if (ImGui::Button("Under")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/UNDER_LEDGE_01");
						svc::playerLocator.get().set_position({player::PLAYER_START_X, player::PLAYER_START_Y});
					}
					if (ImGui::Button("House")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/UNDER_HUT_01");

						svc::playerLocator.get().set_position({100, 160});
					}
					if (ImGui::Button("Ancient Field")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/ANCIENT_FIELD_01");

						svc::playerLocator.get().set_position({100, 160});
					}
					if (ImGui::Button("Base")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/BASE_LIVING_01");

						svc::playerLocator.get().set_position({25 * 32, 10 * 32});
					}
					if (ImGui::Button("Base Lab")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/BASE_LAB_01");

						svc::playerLocator.get().set_position({28 * 32, 8 * 32});
					}
					if (ImGui::Button("Skycorps")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/SKYCORPS_ANTECHAMBER_01");

						svc::playerLocator.get().set_position({28 * 32, 8 * 32});
					}
					if (ImGui::Button("Sky")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/SKY_CHAMBER_01");
						svc::playerLocator.get().set_position({7 * 32, 16 * 32});
					}
					if (ImGui::Button("Shadow")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/SHADOW_DOJO_01");
						svc::playerLocator.get().set_position({4 * 32, 11 * 32});
					}
					if (ImGui::Button("Stone")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/STONE_CORRIDOR_01");
						svc::playerLocator.get().set_position({10 * 32, 16 * 32});
					}
					if (ImGui::Button("Overturned")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/OVERTURNED_DOJO_01");
						svc::playerLocator.get().set_position({4 * 32, 11 * 32});
					}
					if (ImGui::Button("Glade")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/OVERTURNED_GLADE_01");
						svc::playerLocator.get().set_position({4 * 32, 4 * 32});
					}
					if (ImGui::Button("Woodshine")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/WOODSHINE_VILLAGE_01");
						svc::playerLocator.get().set_position({32, 1280});
					}
					if (ImGui::Button("Collision Room")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/SKY_COLLISIONROOM_01");
						svc::playerLocator.get().set_position({5 * 32, 5 * 32});
					}
					if (ImGui::Button("Grub Dojo")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/GRUB_DOJO_01");
						svc::playerLocator.get().set_position({3 * 32, 8 * 32});
					}
					if (ImGui::Button("Firstwind Dojo")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/FIRSTWIND_DOJO_01");
						svc::playerLocator.get().set_position({3 * 32, 8 * 32});
					}
					/*if (ImGui::Button("Atrium")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/FIRSTWIND_ATRIUM_01");
					}
					if (ImGui::Button("Hangar")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/FIRSTWIND_HANGAR_01");
						svc::playerLocator.get().set_position({ 3080, 790 });
					}
					if (ImGui::Button("Corridor 3")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/FIRSTWIND_CORRIDOR_03");
						svc::playerLocator.get().set_position({ 2327, 360 });
					}
					if(ImGui::Button("Lab")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/TOXIC_LAB_01");
					}*/
					if (ImGui::Button("Toxic")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/TOXIC_ARENA_01");
						svc::playerLocator.get().set_position({player::PLAYER_START_X, player::PLAYER_START_Y});
						svc::playerLocator.get().collider.physics.zero();
						svc::playerLocator.get().set_position({34, 484});
					}
					if (ImGui::Button("Grub")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/GRUB_TUNNEL_01");
						svc::playerLocator.get().set_position({224, 290});
					}
					/*if(ImGui::Button("Night")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().resource_path + "/level/NIGHT_CRANE_01");
						svc::playerLocator.get().set_position({50, 50});
						svc::playerLocator.get().assign_texture(svc::assetLocator.get().t_nani_dark);
					}*/
					if (ImGui::Button("Night 2")) {
						svc::assetLocator.get().click.play();
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/NIGHT_CATWALK_01");
						svc::playerLocator.get().set_position({50, 50});
						svc::playerLocator.get().assign_texture(svc::assetLocator.get().t_nani_dark);
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImVec2 prev_size = ImGui::GetWindowSize();
		ImGui::End();
	}
}

void run(char** argv) {

	// load all assets
	// data
	svc::dataLocator.get().finder.setResourcePath(argv);
	svc::dataLocator.get().load_data();
	//text
	svc::textLocator.get().finder.setResourcePath(argv);
	svc::textLocator.get().load_data();
	// images
	svc::assetLocator.get().finder.setResourcePath(argv);
	svc::assetLocator.get().importTextures();
	svc::assetLocator.get().assignSprites();
	// sounds
	svc::assetLocator.get().load_audio();
	// player
	svc::playerLocator.get().init();

	// state manager
	SM.set_current_state(std::make_unique<automa::MainMenu>());
	SM.get_current_state().init(svc::assetLocator.get().finder.resource_path);

	window.create(sf::VideoMode(cam::screen_dimensions.x, cam::screen_dimensions.y), "For Nani (beta v1.0)");

	screencap.create(window.getSize().x, window.getSize().y);

	bool debug_mode = false;

	// some SFML variables for drawing a basic window + background
	window.setVerticalSyncEnabled(true);
	// window.setFramerateLimit(20);
	window.setKeyRepeatEnabled(false);

	ImGui::SFML::Init(window);
	window.requestFocus();

	// lookups
	lookup::populate_lookup();

	sf::RectangleShape background{};
	background.setSize(static_cast<sf::Vector2<float>>(cam::screen_dimensions));
	background.setPosition(0, 0);
	background.setFillColor(sf::Color(10, 10, 20));

	width_ratio = (float)cam::screen_dimensions.x / (float)cam::screen_dimensions.y;
	height_ratio = (float)cam::screen_dimensions.y / (float)cam::screen_dimensions.x;

	// game loop
	sf::Clock deltaClock{};

	while (window.isOpen()) {

		svc::tickerLocator.get().start_frame();

		win_size.x = window.getSize().x;
		win_size.y = window.getSize().y;

		uint16_t draw_counter = 0;
		svc::counterLocator.get().at(svc::draw_calls) = draw_counter;

		// SFML event variable
		auto event = sf::Event{};

		bool valid_event{};
		// check window events
		while (window.pollEvent(event)) {
			svc::playerLocator.get().animation.state = {};
			if (event.key.code == sf::Keyboard::F2) { valid_event = false; }
			if (event.key.code == sf::Keyboard::F3) { valid_event = false; }
			if (event.key.code == sf::Keyboard::Slash) { valid_event = false; }
			switch (event.type) {
			case sf::Event::Closed: return;
			case sf::Event::Resized:
				win_size.x = window.getSize().x;
				win_size.y = window.getSize().y;
				if (win_size.y * width_ratio <= win_size.x) {
					win_size.x = win_size.y * width_ratio;
				} else if (win_size.x * height_ratio <= win_size.y) {
					win_size.y = win_size.x * height_ratio;
				}
				window.setSize(sf::Vector2u{win_size.x, win_size.y});
				screencap.create(window.getSize().x, window.getSize().y);
				break;
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::F2) { valid_event = false; }
				if (event.key.code == sf::Keyboard::F3) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Slash) { valid_event = false; }
				if (event.key.code == sf::Keyboard::Escape) { return; }
				if (event.key.code == sf::Keyboard::D) {
					debug_mode = !debug_mode;
					svc::assetLocator.get().click.play();
				}
				if (event.key.code == sf::Keyboard::K) { svc::playerLocator.get().kill(); }
				if (event.key.code == sf::Keyboard::T) {
					svc::consoleLocator.get().load_and_launch("bookshelf_1");
				}
				if (event.key.code == sf::Keyboard::Q) { SM.set_current_state(std::make_unique<automa::MainMenu>()); }
				if (event.key.code == sf::Keyboard::W) {
					SM.set_current_state(std::make_unique<automa::Dojo>());
					SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/FIRSTWIND_PRISON_01");
					svc::assetLocator.get().dusken_cove.setVolume(svc::assetLocator.get().music_vol);
					// svc::assetLocator.get().dusken_cove.play();
					svc::assetLocator.get().dusken_cove.setLoop(true);
					svc::playerLocator.get().assign_texture(svc::assetLocator.get().t_nani);
				}
				if (event.key.code == sf::Keyboard::P) { save_screenshot(); }
				break;
			case sf::Event::KeyReleased: break;
			default: break;
			}
			SM.get_current_state().handle_events(event);
			if (valid_event) { ImGui::SFML::ProcessEvent(event); }
			valid_event = true;
		}

		// game logic and rendering
		svc::tickerLocator.get().tick([] { SM.get_current_state().tick_update(); });
		SM.get_current_state().frame_update();

		//play sounds
		svc::soundboardLocator.get().play_sounds();
		
		SM.get_current_state().debug_mode = debug_mode;

		// switch states
		if (svc::stateControllerLocator.get().trigger_submenu) {
			switch (svc::stateControllerLocator.get().submenu) {
			case automa::menu_type::file_select: SM.set_current_state(std::make_unique<automa::FileMenu>()); break;
			case automa::menu_type::options:
				// todo
				break;
			case automa::menu_type::settings:
				// todo
				break;
			case automa::menu_type::credits:
				// todo
				break;
			}
			svc::stateControllerLocator.get().trigger_submenu = false;
		}
		if (svc::stateControllerLocator.get().exit_submenu) {
			SM.set_current_state(std::make_unique<automa::MainMenu>());
			svc::stateControllerLocator.get().exit_submenu = false;
		}
		if (svc::stateControllerLocator.get().trigger) {
			SM.set_current_state(std::make_unique<automa::Dojo>());
			SM.get_current_state().init(svc::assetLocator.get().finder.resource_path + "/level/" + svc::stateControllerLocator.get().next_state);
			svc::stateControllerLocator.get().trigger = false;
		}

		// reset global triggers
		svc::globalBitFlagsLocator.get().reset(svc::global_flags::greyblock_trigger);

		ImGui::SFML::Update(window, deltaClock.restart());
		screencap.update(window);

		// ImGui stuff
		if (debug_mode) { show_overlay(); }

		// my renders
		window.clear();
		window.draw(background);

		SM.get_current_state().render(window);

		ImGui::SFML::Render(window);
		window.display();
		frame_draw_counter = svc::counterLocator.get().at(svc::draw_calls);
		svc::tickerLocator.get().end_frame();
	}
}

} // namespace

int main(int argc, char** argv) {
	assert(argc > 0);
	run(argv);
	ImGui::SFML::Shutdown();
	return 0;
}
