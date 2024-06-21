
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {

enum class Action { left, right, up, down, main_action, secondary_action, tertiary_action, inspect, sprint, shield, arms_switch_left, arms_switch_right, menu_toggle, menu_toggle_secondary, menu_forward, menu_back };
enum class ActionState { held, released, triggered };
enum class ControllerType { keyboard, gamepad };
enum class Toggles { keyboard, gamepad };

struct Control {
	Control(Action action) : action(action) {}
	Action action{};
	util::BitFlags<ActionState> state{};
	[[nodiscard]] auto held() const -> bool { return state.test(ActionState::held); }
	[[nodiscard]] auto released() const -> bool { return state.test(ActionState::released); }
	[[nodiscard]] auto triggered() const -> bool { return state.test(ActionState::triggered); }
	void press() {
		state.set(ActionState::triggered);
		state.set(ActionState::held);
	}
	void release() {
		state.set(ActionState::released);
		state.reset(ActionState::held);
	}
	void reset_triggers() {
		state.reset(ActionState::triggered);
		state.reset(ActionState::released);
	}
};

class ControllerMap {
  public:
	ControllerMap(automa::ServiceProvider& svc);
	void handle_mouse_events(sf::Event& event);
	void handle_press(sf::Keyboard::Key& k);
	void handle_release(sf::Keyboard::Key& k);
	void handle_joystick_events(sf::Event& event);
	void reset_triggers();
	void switch_to_joystick();
	void switch_to_keyboard();
	[[nodiscard]] auto get_throttle() const -> sf::Vector2<float> { return throttle; }
	[[nodiscard]] auto is_gamepad() const -> bool { return type == ControllerType::gamepad; }
	[[nodiscard]] auto is_keyboard() const -> bool { return type == ControllerType::keyboard; }
	[[nodiscard]] auto joystick_moved() const -> bool { return throttle.x < -throttle_threshold || throttle.x > throttle_threshold || throttle.y < -throttle_threshold || throttle.y > throttle_threshold; }

	std::vector<std::string_view> tags{"main_action", "secondary_action", "tertiary_action", "inspect", "sprint", "shield", "menu_toggle", "menu_toggle_secondary", "arms_switch_left", "arms_switch_right", "left", "right", "up",
									   "down", "menu_forward", "menu_back"};
	std::unordered_map<std::string_view, Control> label_to_control{};
	std::unordered_map<std::string_view, std::string_view> tag_to_label{};
	std::unordered_map<sf::Keyboard::Key, std::string_view> key_to_label{};
	std::unordered_map<sf::Mouse::Button, std::string_view> mousebutton_to_label{};
	std::unordered_map<int, std::string_view> gamepad_button_name{};
	std::unordered_map<std::string_view, int> label_to_gamepad{};
	std::unordered_map<std::string_view, sf::Keyboard::Key> string_to_key{{"A", sf::Keyboard::A},			{"B", sf::Keyboard::B},
																		  {"C", sf::Keyboard::C},			{"D", sf::Keyboard::D},
																		  {"E", sf::Keyboard::E},			{"F", sf::Keyboard::F},
																		  {"G", sf::Keyboard::G},			{"H", sf::Keyboard::H},
																		  {"I", sf::Keyboard::I},			{"J", sf::Keyboard::J},
																		  {"K", sf::Keyboard::K},			{"L", sf::Keyboard::L},
																		  {"M", sf::Keyboard::M},			{"N", sf::Keyboard::N},
																		  {"O", sf::Keyboard::O},			{"P", sf::Keyboard::P},
																		  {"Q", sf::Keyboard::Q},			{"R", sf::Keyboard::R},
																		  {"S", sf::Keyboard::S},			{"T", sf::Keyboard::T},
																		  {"U", sf::Keyboard::U},			{"V", sf::Keyboard::V},
																		  {"W", sf::Keyboard::W},			{"X", sf::Keyboard::X},
																		  {"Y", sf::Keyboard::Y},			{"Z", sf::Keyboard::Z},
																		  {"LShift", sf::Keyboard::LShift}, {"RShift", sf::Keyboard::RShift},
																		  {"Left", sf::Keyboard::Left},		{"Right", sf::Keyboard::Right},
																		  {"Up", sf::Keyboard::Up},			{"Down", sf::Keyboard::Down},
																		  {"Period", sf::Keyboard::Period}, {"1", sf::Keyboard::Num1},
																		  {"2", sf::Keyboard::Num2},		{"3", sf::Keyboard::Num3},
																		  {"Space", sf::Keyboard::Space},	{"LControl", sf::Keyboard::LControl},
																		  {"Esc", sf::Keyboard::Escape},	{"Enter", sf::Keyboard::Enter}};
	std::unordered_map<sf::Keyboard::Key, std::string_view> key_to_string{{sf::Keyboard::A, "A"},			{sf::Keyboard::B, "B"},
																		  {sf::Keyboard::C, "C"},			{sf::Keyboard::D, "D"},
																		  {sf::Keyboard::E, "E"},			{sf::Keyboard::F, "F"},
																		  {sf::Keyboard::G, "G"},			{sf::Keyboard::H, "H"},
																		  {sf::Keyboard::I, "I"},			{sf::Keyboard::J, "J"},
																		  {sf::Keyboard::K, "K"},			{sf::Keyboard::L, "L"},
																		  {sf::Keyboard::M, "M"},			{sf::Keyboard::N, "N"},
																		  {sf::Keyboard::O, "O"},			{sf::Keyboard::P, "P"},
																		  {sf::Keyboard::Q, "Q"},			{sf::Keyboard::R, "R"},
																		  {sf::Keyboard::S, "S"},			{sf::Keyboard::T, "T"},
																		  {sf::Keyboard::U, "U"},			{sf::Keyboard::V, "V"},
																		  {sf::Keyboard::W, "W"},			{sf::Keyboard::X, "X"},
																		  {sf::Keyboard::Y, "Y"},			{sf::Keyboard::Z, "Z"},
																		  {sf::Keyboard::LShift, "LShift"}, {sf::Keyboard::RShift, "RShift"},
																		  {sf::Keyboard::Left, "Left"},		{sf::Keyboard::Right, "Right"},
																		  {sf::Keyboard::Up, "Up"},			{sf::Keyboard::Down, "Down"},
																		  {sf::Keyboard::Period, "Period"}, {sf::Keyboard::Num1, "1"},
																		  {sf::Keyboard::Num2, "2"},		{sf::Keyboard::Num3, "3"},
																		  {sf::Keyboard::Space, "Space"},	{sf::Keyboard::LControl, "LControl"},
																		  {sf::Keyboard::Escape, "Esc"}};
	std::unordered_map<std::string_view, sf::Mouse::Button> string_to_mousebutton{{"LMB", sf::Mouse::Left}, {"RMB", sf::Mouse::Right}};

	ControllerType type{};
	util::BitFlags<Toggles> hard_toggles{};

  private:
	sf::Vector2<float> throttle{};
	float const throttle_threshold{0.4f};
};

} // namespace config
