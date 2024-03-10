
// main

#include <SFML/Graphics.hpp>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>

#include "src/automa/StateManager.hpp"
#include "src/setup/EnumLookups.hpp"
#include "src/setup/ServiceLocator.hpp"
#include "src/level/Grid.hpp"
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
bool music{};

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

static void show_overlay(services::ServiceLocator& svc) {
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
				if (ImGui::BeginTabItem("Logger")) {
					ImGui::Separator();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Time")) {
					ImGui::Separator();
					ImGui::Text("Ticker");
					ImGui::Text("dt: %.8f", svc.tickerLocator.get().dt.count());
					ImGui::Text("New Time: %.4f", svc.tickerLocator.get().new_time);
					ImGui::Text("Current Time: %.4f", svc.tickerLocator.get().current_time);
					ImGui::Text("Accumulator: %.4f", svc.tickerLocator.get().accumulator.count());
					ImGui::Separator();
					ImGui::Text("Seconds Passed: %.2f", svc.tickerLocator.get().total_seconds_passed.count());
					ImGui::Text("Ticks Per Frame: %.2f", svc.tickerLocator.get().ticks_per_frame);
					ImGui::Text("Frames Per Second: %.2f", svc.tickerLocator.get().fps);
					ImGui::Separator();
					ImGui::SliderFloat("Tick Rate (ms): ", &svc.tickerLocator.get().tick_rate, 0.0001f, 0.02f, "%.5f");
					ImGui::SliderFloat("Tick Multiplier: ", &svc.tickerLocator.get().tick_multiplier, 0.f, 64.f, "%.2f");
					if (ImGui::Button("Reset")) {
						svc.tickerLocator.get().tick_rate = 0.016f;
						svc.tickerLocator.get().tick_multiplier = 16;
					}
					ImGui::Separator();
					ImGui::Text("Stopwatch");
					ImGui::Text("average time: %.4f", svc.stopwatchLocator.get().get_snapshot());

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Console")) {
					ImGui::Separator();

					ImGui::Text("Console Active : %s", SM.get_current_state().console.flags.test(gui::ConsoleFlags::active) ? "Yes" : "No");
					ImGui::Text("Console Writer Active : %s", SM.get_current_state().console.writer.active() ? "Yes" : "No");
					ImGui::Text("Console Writer Complete : %s", SM.get_current_state().console.writer.complete() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Player Transponder Skipping : %s", SM.get_current_state().player.transponder.skipped_ahead() ? "Yes" : "No");
					ImGui::Text("Player Transponder Exited : %s", SM.get_current_state().player.transponder.exited() ? "Yes" : "No");
					ImGui::Text("Player Transponder Requested Next : %s", SM.get_current_state().player.transponder.requested_next() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Player Restricted? : %s", SM.get_current_state().player.controller.restricted() ? "Yes" : "No");
					ImGui::Text("Player Inspecting? : %s", SM.get_current_state().player.controller.inspecting() ? "Yes" : "No");
					ImGui::Separator();
					ImGui::SliderInt("Text Size", &SM.get_current_state().console.writer.text_size, 6, 64);

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Key States")) {
					ImGui::Text("Shift held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Shift triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Shift released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::LShift).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Left held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Left triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Left released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Right held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Right triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Right released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Up held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Up triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Up released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Down held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Down triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Down released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Z held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("Z triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("Z released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("X held: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::held) ? "Yes" : "No");
					ImGui::Text("X triggered: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::triggered) ? "Yes" : "No");
					ImGui::Text("X released: %s", svc.inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::released) ? "Yes" : "No");
					ImGui::Separator();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Audio")) {
					ImGui::Separator();
					ImGui::Text("Music Player");
					if (ImGui::Checkbox("music", &music)) { music ? svc.musicPlayerLocator.get().turn_on() : svc.musicPlayerLocator.get().turn_off(); }
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Player")) {
					if (ImGui::BeginTabBar("PlayerTabBar", tab_bar_flags)) {
						if (ImGui::BeginTabItem("Texture")) {
							if (ImGui::Button("Default")) { SM.get_current_state().player.texture_updater.switch_to_palette(svc.assetLocator.get().t_palette_nani); }
							if (ImGui::Button("Divine")) { SM.get_current_state().player.texture_updater.switch_to_palette(svc.assetLocator.get().t_palette_nanidiv); }
							if (ImGui::Button("Night")) { SM.get_current_state().player.texture_updater.switch_to_palette(svc.assetLocator.get().t_palette_naninight); }
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Physics and Collision")) {
							ImGui::Text("Player Pos: (%.4f,%.4f)", SM.get_current_state().player.collider.physics.position.x, SM.get_current_state().player.collider.physics.position.y);
							ImGui::Text("Player Vel: (%.4f,%.4f)", SM.get_current_state().player.collider.physics.velocity.x, SM.get_current_state().player.collider.physics.velocity.y);
							ImGui::Text("Player Acc: (%.4f,%.4f)", SM.get_current_state().player.collider.physics.acceleration.x, SM.get_current_state().player.collider.physics.acceleration.y);
							ImGui::Text("Player Jer: (%.4f,%.4f)", SM.get_current_state().player.collider.physics.jerk.x, SM.get_current_state().player.collider.physics.jerk.y);
							ImGui::Separator();
							ImGui::Text("Player Grounded: %s", SM.get_current_state().player.grounded() ? "Yes" : "No");
							ImGui::Separator();
							ImGui::Text("Right Collision: %s", SM.get_current_state().player.collider.collision_flags.test(shape::Collision::has_right_collision) ? "Yes" : "No");
							ImGui::Text("Left Collision: %s", SM.get_current_state().player.collider.collision_flags.test(shape::Collision::has_left_collision) ? "Yes" : "No");
							ImGui::Text("Top Collision: %s", SM.get_current_state().player.collider.collision_flags.test(shape::Collision::has_top_collision) ? "Yes" : "No");
							ImGui::Text("Bottom Collision: %s", SM.get_current_state().player.collider.collision_flags.test(shape::Collision::has_bottom_collision) ? "Yes" : "No");
							ImGui::Separator();
							ImGui::Text("Dash Cancel Collision: %s", SM.get_current_state().player.collider.dash_flags.test(shape::Dash::dash_cancel_collision) ? "Yes" : "No");

							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Movement")) {
							ImGui::Text("Direction LR	: %s", SM.get_current_state().player.controller.direction.print_lr().c_str());
							ImGui::Text("Direction UND	: %s", SM.get_current_state().player.controller.direction.print_und().c_str());
							ImGui::Separator();
							ImGui::Text("Controller");
							ImGui::Text("Move Left : %s", SM.get_current_state().player.controller.get_controller_state(player::ControllerInput::move_x) < 0.f ? "Yes" : "No");
							ImGui::Text("Move Right : %s", SM.get_current_state().player.controller.get_controller_state(player::ControllerInput::move_x) > 0.f ? "Yes" : "No");
							ImGui::Text("Inspecting : %s", SM.get_current_state().player.controller.get_controller_state(player::ControllerInput::inspect) > 0.f ? "Yes" : "No");
							ImGui::Text("Facing : %s", SM.get_current_state().player.print_direction(true).c_str());
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Jump")) {
							ImGui::Text("Jump Request : %i", SM.get_current_state().player.controller.get_jump().get_request());
							ImGui::Text("Jump Released : %s", SM.get_current_state().player.controller.get_jump().released() ? "Yes" : "No");
							ImGui::Text("Can Jump : %s", SM.get_current_state().player.controller.get_jump().can_jump() ? "Yes" : "No");
							ImGui::Text("Jumping? : %s", SM.get_current_state().player.controller.get_jump().jumping() ? "Yes" : "No");
							ImGui::Text("Jump Began? : %s", SM.get_current_state().player.controller.get_jump().began() ? "Yes" : "No");
							ImGui::Text("Jumping? : %s", SM.get_current_state().player.collider.movement_flags.test(shape::Movement::jumping) ? "Yes" : "No");
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Dash")) {
							ImGui::Text("Dash Value : %f", SM.get_current_state().player.controller.dash_value());
							ImGui::Text("Dash Request : %i", SM.get_current_state().player.controller.get_dash_request());
							ImGui::Text("Dash Count : %i", SM.get_current_state().player.controller.get_dash_count());
							ImGui::Text("Can Dash? : %s", SM.get_current_state().player.controller.can_dash() ? "Yes" : "No");
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Animation")) {
							ImGui::Text("Animation: %s", SM.get_current_state().player.animation.animation.label.c_str());
							ImGui::Separator();
							ImGui::Text("Current Frame: %i", SM.get_current_state().player.animation.animation.current_frame);
							ImGui::Text("Complete? %s", SM.get_current_state().player.animation.animation.complete() ? "Yes" : "No");
							ImGui::Text("One Off? %s", SM.get_current_state().player.animation.animation.params.num_loops > -1 ? "Yes" : "No");
							ImGui::Text("Repeat Last Frame? %s", SM.get_current_state().player.animation.animation.params.repeat_last_frame ? "Yes" : "No");
							ImGui::Separator();
							ImGui::Text("idle...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::idle) ? "flag set" : "");
							ImGui::Text("run....: %s", SM.get_current_state().player.animation.state.test(player::AnimState::run) ? "flag set" : "");
							ImGui::Text("stop...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::stop) ? "flag set" : "");
							ImGui::Text("turn...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::turn) ? "flag set" : "");
							ImGui::Text("jsquat.: %s", SM.get_current_state().player.animation.state.test(player::AnimState::jumpsquat) ? "flag set" : "");
							ImGui::Text("rise...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::rise) ? "flag set" : "");
							ImGui::Text("suspend: %s", SM.get_current_state().player.animation.state.test(player::AnimState::suspend) ? "flag set" : "");
							ImGui::Text("fall...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::fall) ? "flag set" : "");
							ImGui::Text("land...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::land) ? "flag set" : "");
							ImGui::Text("dash...: %s", SM.get_current_state().player.animation.state.test(player::AnimState::dash) ? "flag set" : "");
							ImGui::Text("inspect: %s", SM.get_current_state().player.animation.state.test(player::AnimState::inspect) ? "flag set" : "");
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Stats")) {
							ImGui::Text("Player Stats");
							ImGui::SliderInt("Max HP", &SM.get_current_state().player.player_stats.max_health, 3, 12);
							ImGui::SliderInt("HP", &SM.get_current_state().player.player_stats.health, 3, 12);
							ImGui::SliderInt("Max Orbs", &SM.get_current_state().player.player_stats.max_orbs, 99, 99999);
							ImGui::SliderInt("Orbs", &SM.get_current_state().player.player_stats.orbs, 0, 99999);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Parameter Tweaking")) {
							ImGui::Text("Vertical Movement");
							ImGui::SliderFloat("GRAVITY", &SM.get_current_state().player.physics_stats.grav, 0.f, 0.8f, "%.5f");
							ImGui::SliderFloat("JUMP VELOCITY", &SM.get_current_state().player.physics_stats.jump_velocity, 0.5f, 12.0f, "%.5f");
							ImGui::SliderFloat("JUMP RELEASE MULTIPLIER", &SM.get_current_state().player.physics_stats.jump_release_multiplier, 0.005f, 1.f, "%.5f");
							ImGui::SliderFloat("MAX Y VELOCITY", &SM.get_current_state().player.physics_stats.maximum_velocity.y, 1.0f, 60.0f);

							ImGui::Separator();
							ImGui::Text("Horizontal Movement");
							ImGui::SliderFloat("AIR MULTIPLIER", &SM.get_current_state().player.physics_stats.air_multiplier, 0.0f, 5.0f);
							ImGui::SliderFloat("GROUND FRICTION", &SM.get_current_state().player.physics_stats.ground_fric, 0.800f, 1.000f);
							ImGui::SliderFloat("AIR FRICTION", &SM.get_current_state().player.physics_stats.air_fric, 0.800f, 1.000f);
							ImGui::SliderFloat("GROUND SPEED", &SM.get_current_state().player.physics_stats.x_acc, 0.0f, 3.f);
							ImGui::SliderFloat("MAX X VELOCITY", &SM.get_current_state().player.physics_stats.maximum_velocity.x, 1.0f, 10.0f);

							ImGui::Separator();
							ImGui::Text("Dash");
							ImGui::SliderFloat("Dash Speed", &SM.get_current_state().player.physics_stats.dash_speed, 1.0f, 30.0f);
							ImGui::SliderFloat("Vertical Dash Multiplier", &SM.get_current_state().player.physics_stats.vertical_dash_multiplier, 0.0f, 10.0f);
							ImGui::SliderFloat("Dash Dampen", &SM.get_current_state().player.physics_stats.dash_dampen, 0.7f, 2.0f);

							ImGui::Separator();
							if (ImGui::Button("Save Parameters")) { svc.dataLocator.get().save_player_params()); }
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Misc")) {

							ImGui::Text("Alive? %s", SM.get_current_state().player.flags.state.test(player::State::alive) ? "Yes" : "No");

							ImGui::Text("Invincibility Counter: %i", SM.get_current_state().player.counters.invincibility);
							ImGui::Text("Spike Trigger: %s", SM.get_current_state().player.collider.spike_trigger ? "True" : "False");
							ImGui::Text("On Ramp: %s", SM.get_current_state().player.collider.on_ramp() ? "True" : "False");

							ImGui::Text("Grounded: %s", SM.get_current_state().player.grounded() ? "Yes" : "No");
							ImGui::EndTabItem();
						}

						ImGui::EndTabBar();
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Weapon")) {
					if (ImGui::Button("Toggle Weapons")) {
						if (SM.get_current_state().player.arsenal.loadout.empty()) {
							// SM.get_current_state().player.weapons_hotbar = {arms::WEAPON_TYPE::BRYNS_GUN, arms::WEAPON_TYPE::PLASMER, arms::WEAPON_TYPE::CLOVER, arms::WEAPON_TYPE::NOVA};
							// SM.get_current_state().player.loadout.equipped_weapon = SM.get_current_state().player.weapons_hotbar.at(0);
							SM.get_current_state().player.arsenal.push_to_loadout(lookup::type_to_index.at(arms::WEAPON_TYPE::BRYNS_GUN));
							SM.get_current_state().player.arsenal.push_to_loadout(lookup::type_to_index.at(arms::WEAPON_TYPE::PLASMER));
							SM.get_current_state().player.arsenal.push_to_loadout(lookup::type_to_index.at(arms::WEAPON_TYPE::TOMAHAWK));
						} else {
							// SM.get_current_state().player.weapons_hotbar.clear();
							SM.get_current_state().player.arsenal.loadout = {};
						}
					}

					ImGui::Separator();
					ImGui::Text("Grappling Hook:");
					ImGui::Text("Hook held: %s", SM.get_current_state().player.controller.hook_held() ? "Yes" : "No");
					ImGui::Text("Direction: %s", SM.get_current_state().player.equipped_weapon().projectile.hook.probe_direction.print_intermediate().c_str());

					ImGui::Separator();
					ImGui::Text("Extant Projectiles:");
					ImGui::NewLine();
					for (auto& weapon : SM.get_current_state().player.arsenal.loadout) {
						ImGui::Text("%s", (weapon.label + ": ").c_str());
						ImGui::SameLine();
						ImGui::Text("%i", SM.get_current_state().player.extant_instances(weapon.get_id()));
					}
					ImGui::Separator();

					ImGui::Text("Firing Direction LR: %s", SM.get_current_state().player.equipped_weapon().firing_direction.print_lr().c_str());
					ImGui::Text("Firing Direction UND : %s", SM.get_current_state().player.equipped_weapon().firing_direction.print_und().c_str());

					ImGui::Text("Cooling Down? %s", SM.get_current_state().player.equipped_weapon().cooling_down() ? "Yes" : "No");
					ImGui::Text("Cooldown Time %i", SM.get_current_state().player.equipped_weapon().cooldown_counter);
					ImGui::Text("Active Projectiles: %i", SM.get_current_state().player.equipped_weapon().active_projectiles);

					ImGui::Separator();
					ImGui::Text("Equipped Weapon: %s", SM.get_current_state().player.equipped_weapon().label.c_str());
					ImGui::Text("UI color: %i", (int)SM.get_current_state().player.equipped_weapon().attributes.ui_color);
					ImGui::Text("Sprite Dimensions X: %i", SM.get_current_state().player.equipped_weapon().sprite_dimensions.x);
					ImGui::Text("Sprite Dimensions Y: %i", SM.get_current_state().player.equipped_weapon().sprite_dimensions.y);
					ImGui::Text("Barrel Point X: %.1f", SM.get_current_state().player.equipped_weapon().barrel_point.x);
					ImGui::Text("Barrel Point Y: %.1f", SM.get_current_state().player.equipped_weapon().barrel_point.y);
					ImGui::Separator();
					ImGui::Text("Weapon Stats: ");
					ImGui::Indent();
					ImGui::Text("Rate: (%i)", SM.get_current_state().player.equipped_weapon().attributes.rate);
					ImGui::Text("Cooldown: (%i)", SM.get_current_state().player.equipped_weapon().attributes.cooldown_time);
					ImGui::Text("Recoil: (%.2f)", SM.get_current_state().player.equipped_weapon().attributes.recoil);
					ImGui::Text("Spray Force: (%.2f)", SM.get_current_state().player.equipped_weapon().spray_behavior.expulsion_force);
					ImGui::Text("Spray Grav: (%.2f)", SM.get_current_state().player.equipped_weapon().spray_behavior.grav);

					ImGui::Separator();
					ImGui::Unindent();
					ImGui::Text("Projectile Stats: ");
					ImGui::Indent();
					ImGui::Text("Damage: (%f)", SM.get_current_state().player.equipped_weapon().projectile.stats.base_damage);
					ImGui::Text("Range: (%i)", SM.get_current_state().player.equipped_weapon().projectile.stats.range);
					ImGui::Text("Speed: (%.2f)", SM.get_current_state().player.equipped_weapon().projectile.stats.speed);
					ImGui::Text("Velocity: (%.4f,%.4f)", SM.get_current_state().player.equipped_weapon().projectile.physics.velocity.x, SM.get_current_state().player.equipped_weapon().projectile.physics.velocity.y);
					ImGui::Text("Position: (%.4f,%.4f)", SM.get_current_state().player.equipped_weapon().projectile.physics.position.x, SM.get_current_state().player.equipped_weapon().projectile.physics.position.y);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("General")) {

					if (ImGui::Button("Save Screenshot")) { save_screenshot(); }
					ImGui::Separator();
					if (ImGui::Button("Toggle Greyblock Mode")) {
						if (svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state)) {
							svc.globalBitFlagsLocator.get().reset(services::global_flags::greyblock_state);
							svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_trigger);
						} else {
							svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_state);
							svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_trigger);
						}
					}
					ImGui::Text("Greyblock Mode : %s", svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state) ? "On" : "Off");
					ImGui::Separator();
					ImGui::Text("Draw Calls: %u", frame_draw_counter);
					frame_draw_counter = 0;

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Resources")) {
					ImGui::Text("Size of Asset Manager (Bytes): %lu", sizeof(svc.assetLocator.get()));
					ImGui::Text("Size of Data Manager (Bytes): %lu", sizeof(svc.dataLocator.get()));
					ImGui::Text("Size of Text Manager (Bytes): %lu", sizeof(svc.textLocator.get()));
					ImGui::Text("Size of Music Player (Bytes): %lu", sizeof(svc.musicPlayerLocator.get()));
					ImGui::Text("Size of Player (Bytes): %lu", sizeof(SM.get_current_state().player));
					ImGui::Text("Size of TextureUpdater (Bytes): %lu", sizeof(SM.get_current_state().player.texture_updater));
					ImGui::Text("Size of Collider (Bytes): %lu", sizeof(SM.get_current_state().player.collider));
					ImGui::Text("Size of Arsenal (Bytes): %lu", sizeof(SM.get_current_state().player.arsenal));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("State")) {
					ImGui::Separator();
					ImGui::Text("State");
					ImGui::Text("Current State: ");
					ImGui::SameLine();
					ImGui::TextUnformatted(SM.get_current_state_string().c_str());
					if (ImGui::Button("Under")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/UNDER_LEDGE_01", svc);
					}
					if (ImGui::Button("House")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/UNDER_HUT_01", svc);

						SM.get_current_state().player.set_position({100, 160});
					}
					if (ImGui::Button("Ancient Field")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/ANCIENT_FIELD_01", svc);

						SM.get_current_state().player.set_position({100, 160});
					}
					if (ImGui::Button("Base")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/BASE_LIVING_01", svc);

						SM.get_current_state().player.set_position({25 * 32, 10 * 32});
					}
					if (ImGui::Button("Sky")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/SKY_CHAMBER_01", svc);
						SM.get_current_state().player.set_position({7 * 32, 16 * 32});
					}
					if (ImGui::Button("Shadow")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/SHADOW_DOJO_01", svc);
						SM.get_current_state().player.set_position({4 * 32, 11 * 32});
					}
					if (ImGui::Button("Stone")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/STONE_CORRIDOR_01", svc);
						SM.get_current_state().player.set_position({10 * 32, 16 * 32});
					}
					if (ImGui::Button("Overturned")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/OVERTURNED_DOJO_01", svc);
						SM.get_current_state().player.set_position({4 * 32, 11 * 32});
					}
					if (ImGui::Button("Glade")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/OVERTURNED_GLADE_01", svc);
						SM.get_current_state().player.set_position({4 * 32, 4 * 32});
					}
					if (ImGui::Button("Woodshine")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/WOODSHINE_VILLAGE_01", svc);
						SM.get_current_state().player.set_position({32, 1280});
					}
					if (ImGui::Button("Collision Room")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/SKY_COLLISIONROOM_01", svc);
						SM.get_current_state().player.set_position({5 * 32, 5 * 32});
					}
					if (ImGui::Button("Grub Dojo")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/GRUB_DOJO_01", svc);
						SM.get_current_state().player.set_position({3 * 32, 8 * 32});
					}
					if (ImGui::Button("Firstwind Dojo")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/FIRSTWIND_DOJO_01", svc);
						SM.get_current_state().player.set_position({3 * 32, 8 * 32});
					}
					if (ImGui::Button("Toxic")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/TOXIC_ARENA_01", svc);
						SM.get_current_state().player.set_position({player::PLAYER_START_X, player::PLAYER_START_Y});
						SM.get_current_state().player.collider.physics.zero();
						SM.get_current_state().player.set_position({34, 484});
					}
					if (ImGui::Button("Grub")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/GRUB_TUNNEL_01", svc);
						SM.get_current_state().player.set_position({224, 290});
					}
					if (ImGui::Button("Night 2")) {
						SM.set_current_state(std::make_unique<automa::Dojo>());
						SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/NIGHT_CATWALK_01", svc);
						SM.get_current_state().player.set_position({50, 50});
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

void run(char** argv, services::ServiceLocator& svc) {

	// load all assets
	// data
	svc.dataLocator.get().finder.setResourcePath(argv);
	svc.dataLocator.get().load_data();
	// text
	svc.textLocator.get().finder.setResourcePath(argv);
	svc.textLocator.get().load_data();
	// images
	svc.assetLocator.get().finder.setResourcePath(argv);
	svc.assetLocator.get().importTextures();
	// sounds
	svc.musicPlayerLocator.get().finder.setResourcePath(argv);
	svc.soundboardLocator.get().finder.setResourcePath(argv);
	svc.soundboardLocator.get().load_audio();
	svc.musicPlayerLocator.get().turn_off(); // off by default
	

	// state manager
	SM.set_current_state(std::make_unique<automa::MainMenu>());
	SM.get_current_state().init(svc.assetLocator.get().finder.resource_path, svc);

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

		svc.tickerLocator.get().start_frame();
		// reset global triggers
		svc.globalBitFlagsLocator.get().reset(services::global_flags::greyblock_trigger);

		win_size.x = window.getSize().x;
		win_size.y = window.getSize().y;

		uint16_t draw_counter = 0;
		svc.counterLocator.get().at(services::draw_calls) = draw_counter;

		// SFML event variable
		auto event = sf::Event{};

		bool valid_event{};
		// check window events
		while (window.pollEvent(event)) {
			SM.get_current_state().player.animation.state = {};
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
					svc.soundboardLocator.get().click.play();
				}
				if (event.key.code == sf::Keyboard::K) { SM.get_current_state().player.kill(); }
				if (event.key.code == sf::Keyboard::T) { SM.get_current_state().console.load_and_launch("bookshelf_1", svc); }
				if (event.key.code == sf::Keyboard::Q) { SM.set_current_state(std::make_unique<automa::MainMenu>()); }
				if (event.key.code == sf::Keyboard::W) {
					SM.set_current_state(std::make_unique<automa::Dojo>());
					SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/FIRSTWIND_PRISON_01", svc);
					SM.get_current_state().player.assign_texture(svc.assetLocator.get().t_nani);
				}
				if (event.key.code == sf::Keyboard::P) { save_screenshot(); }
				if (event.key.code == sf::Keyboard::H) {
					if (svc.globalBitFlagsLocator.get().test(services::global_flags::greyblock_state)) {
						svc.globalBitFlagsLocator.get().reset(services::global_flags::greyblock_state);
						svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_trigger);
					} else {
						svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_state);
						svc.globalBitFlagsLocator.get().set(services::global_flags::greyblock_trigger);
					}
				}
				break;
			case sf::Event::KeyReleased: break;
			default: break;
			}
			SM.get_current_state().handle_events(event, svc);
			if (valid_event) { ImGui::SFML::ProcessEvent(event); }
			valid_event = true;
		}

		// game logic and rendering
		svc.tickerLocator.get().pretick();
		while (svc.tickerLocator.get().accumulator >= svc.tickerLocator.get().ft) {

			SM.get_current_state().tick_update(svc);
			svc.tickerLocator.get().accumulator -= svc.tickerLocator.get().ft;
		}
		svc.tickerLocator.get().posttick();
		SM.get_current_state().frame_update(svc);

		// play sounds
		svc.soundboardLocator.get().play_sounds();

		SM.get_current_state().debug_mode = debug_mode;

		// switch states
		if (svc.stateControllerLocator.get().trigger_submenu) {
			switch (svc.stateControllerLocator.get().submenu) {
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
			svc.stateControllerLocator.get().trigger_submenu = false;
		}
		if (svc.stateControllerLocator.get().exit_submenu) {
			SM.set_current_state(std::make_unique<automa::MainMenu>());
			svc.stateControllerLocator.get().exit_submenu = false;
		}
		if (svc.stateControllerLocator.get().trigger) {
			SM.set_current_state(std::make_unique<automa::Dojo>());
			SM.get_current_state().init(svc.assetLocator.get().finder.resource_path + "/level/" + svc.stateControllerLocator.get().next_state, svc);
			svc.stateControllerLocator.get().trigger = false;
		}

		ImGui::SFML::Update(window, deltaClock.restart());
		screencap.update(window);

		// ImGui stuff
		if (debug_mode) { show_overlay(svc); }

		// my renders
		window.clear();
		window.draw(background);

		SM.get_current_state().render(window, svc);

		ImGui::SFML::Render(window);
		window.display();
		frame_draw_counter = svc.counterLocator.get().at(services::draw_calls);
		svc.tickerLocator.get().end_frame();
	}
}

} // namespace

int main(int argc, char** argv) {
	assert(argc > 0);
	services::ServiceLocator svc{};
	run(argv, svc);
	ImGui::SFML::Shutdown();
	return 0;
}
