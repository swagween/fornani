
#pragma once

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {

enum class DigitalAction {
	// Platformer controls
	platformer_left,
	platformer_right,
	platformer_up,
	platformer_down,
	platformer_jump,
	platformer_shoot,
	platformer_sprint,
	platformer_shield,
	platformer_inspect,
	platformer_arms_switch_left,
	platformer_arms_switch_right,
	platformer_open_inventory,
	platformer_open_map,
	platformer_toggle_pause,

	// Inventory controls
	inventory_left,
	inventory_right,
	inventory_up,
	inventory_down,
	inventory_close,

	// Map controls
	map_close,

	// Menu controls
	menu_up,
	menu_down,
	menu_select,
	menu_cancel
};

enum class AnalogAction {
	// Map controls
	map_movement,
};

enum class ActionSet {
	Platformer,
	Inventory,
	Map,
	Menu,
};

enum class ControllerType { keyboard, gamepad };
enum class ControllerStatus { gamepad_connected };
enum class Toggles { keyboard, gamepad, autosprint };

struct DigitalActionStatus {
	DigitalActionStatus(DigitalAction action) : action(action) {}
	DigitalAction action{};
	bool held{};
	bool triggered{};
	bool released{};
};

struct AnalogActionStatus {
	AnalogActionStatus(AnalogAction action) : action(action) {}
	AnalogAction action{};

	float x{};
	float y{};
};

class ControllerMap {
  public:
	ControllerMap(automa::ServiceProvider& svc);

	ControllerMap(ControllerMap const&) = delete;
	ControllerMap operator=(ControllerMap const&) = delete;

	void update();
	[[nodiscard]] auto gamepad_connected() const -> bool { return status.test(ControllerStatus::gamepad_connected); }
	[[nodiscard]] auto autosprint() const -> bool { return hard_toggles.test(Toggles::autosprint); }
	[[nodiscard]] auto hard_toggles_off() const -> bool { return !hard_toggles.test(Toggles::keyboard) && !hard_toggles.test(Toggles::gamepad); }
	[[nodiscard]] auto digital_action_status(DigitalAction action) const -> DigitalActionStatus { return digital_actions.at(action).second; }
	[[nodiscard]] auto analog_action_status(AnalogAction action) const -> AnalogActionStatus { return analog_actions.at(action).second; }
	[[nodiscard]] auto digital_action_name(DigitalAction action) const -> std::string_view;
	void set_action_set(ActionSet set);

	std::unordered_map<int, std::string_view> gamepad_button_name{};
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
	util::BitFlags<ControllerStatus> status{};

  private:
	std::unordered_map<DigitalAction, std::pair<InputDigitalActionHandle_t, DigitalActionStatus>> digital_actions{};
	std::unordered_map<AnalogAction, std::pair<InputAnalogActionHandle_t, AnalogActionStatus>> analog_actions{};
	std::unordered_map<ActionSet, InputActionSetHandle_t> action_sets{};

	InputHandle_t controller_handle{};

	STEAM_CALLBACK(ControllerMap, handle_gamepad_connection, SteamInputDeviceConnected_t);
};

} // namespace config
