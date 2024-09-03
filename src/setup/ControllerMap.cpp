#include "ControllerMap.hpp"
#include <steam/isteaminput.h>
#include <iostream>
#include "../service/ServiceProvider.hpp"

namespace config {

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {
	std::cout << "Initializing Steam Input" << std::endl;
	if (!SteamInput()->Init(true)) {
		std::cout << "Could not initialize Steam Input!" << std::endl;
	} else {
		std::cout << "Steam Input initialized" << std::endl;
	}
	// SteamInput()->SetInputActionManifestFilePath("C:\\Program Files (x86)\\Steam\\controller_config\\steam_input_manifest.vdf");
	InputHandle_t connected_controllers[STEAM_INPUT_MAX_COUNT];
	SteamInput()->EnableDeviceCallbacks();

#define XSTR(a) STR(a)
#define STR(a) #a
#define DEFINE_ACTION(action_name)                                                                                                                                                                                                             \
	digital_actions.insert({DigitalAction::action_name, {SteamInput()->GetDigitalActionHandle(XSTR(action_name)), DigitalActionStatus(DigitalAction::action_name), sf::Keyboard::Key::Unknown, sf::Keyboard::Key::Unknown}})

	// Platformer controls
	DEFINE_ACTION(platformer_left);
	DEFINE_ACTION(platformer_right);
	DEFINE_ACTION(platformer_up);
	DEFINE_ACTION(platformer_down);
	DEFINE_ACTION(platformer_jump);
	DEFINE_ACTION(platformer_shoot);
	DEFINE_ACTION(platformer_sprint);
	DEFINE_ACTION(platformer_shield);
	DEFINE_ACTION(platformer_inspect);
	DEFINE_ACTION(platformer_arms_switch_left);
	DEFINE_ACTION(platformer_arms_switch_right);
	DEFINE_ACTION(platformer_open_inventory);
	DEFINE_ACTION(platformer_open_map);
	DEFINE_ACTION(platformer_toggle_pause);

	// Inventory controls
	DEFINE_ACTION(inventory_left);
	DEFINE_ACTION(inventory_right);
	DEFINE_ACTION(inventory_up);
	DEFINE_ACTION(inventory_down);
	DEFINE_ACTION(inventory_close);

	// Map controls
	DEFINE_ACTION(map_close);

	// Menu controls
	DEFINE_ACTION(menu_up);
	DEFINE_ACTION(menu_down);
	DEFINE_ACTION(menu_select);
	DEFINE_ACTION(menu_cancel);

	// Analog actions
	analog_actions.insert({AnalogAction::map_movement, {SteamInput()->GetAnalogActionHandle("map_movement"), AnalogActionStatus(AnalogAction::map_movement)}});

#undef DEFINE_ACTION
#define DEFINE_ACTION_SET(set_name) action_sets.insert({ActionSet::set_name, SteamInput()->GetActionSetHandle(XSTR(set_name))})

	DEFINE_ACTION_SET(Platformer);
	DEFINE_ACTION_SET(Menu);
	DEFINE_ACTION_SET(Inventory);
	DEFINE_ACTION_SET(Map);

#undef DEFINE_ACTION_SET
#undef STR
#undef XSTR

	gamepad_button_name.insert({-1, "left analog stick"});
	gamepad_button_name.insert({0, "square"});
	gamepad_button_name.insert({1, "cross"});
	gamepad_button_name.insert({2, "circle"});
	gamepad_button_name.insert({3, "triangle"});
	gamepad_button_name.insert({4, "L1"});
	gamepad_button_name.insert({5, "R1"});
	gamepad_button_name.insert({6, "L2"});
	gamepad_button_name.insert({7, "R2"});
	gamepad_button_name.insert({8, "select"});
	gamepad_button_name.insert({9, "start"});
	gamepad_button_name.insert({10, "left analog click"});
	gamepad_button_name.insert({11, "right analog click"});
	gamepad_button_name.insert({12, "launch"});
	gamepad_button_name.insert({13, "home"});
	gamepad_button_name.insert({14, "unknown"});
	gamepad_button_name.insert({15, "unknown"});
	gamepad_button_name.insert({16, "unknown"});

	hard_toggles.set(Toggles::keyboard);
	hard_toggles.set(Toggles::gamepad);
}

void ControllerMap::update() {
	SteamInput()->RunFrame();
	// SteamInput()->ActivateActionSet(STEAM_INPUT_HANDLE_ALL_CONTROLLERS, SteamInput()->GetActionSetHandle("MenuControls"));
	for (auto& [action, pair] : digital_actions) {
		auto& [steam_handle, action_status, primary_key, secondary_key] = pair;

		auto const data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
		auto pressed_on_gamepad = data.bActive && data.bState;
		auto pressed_on_keyboard = sf::Keyboard::isKeyPressed(primary_key) || sf::Keyboard::isKeyPressed(secondary_key);
		if (pressed_on_gamepad || pressed_on_keyboard) {
			if (!action_status.held) {
				std::cout << "Pressed " << SteamInput()->GetStringForDigitalActionName(steam_handle) << std::endl;
				action_status.triggered = true;
			} else {
				action_status.triggered = false;
			}
			action_status.held = true;
		} else {
			if (action_status.held) {
				action_status.released = true;
			} else {
				action_status.released = false;
			}
			action_status.held = false;
		}
	}

	for (auto& [action, pair] : analog_actions) {
		auto& [steam_handle, action_status] = pair;

		auto const data = SteamInput()->GetAnalogActionData(controller_handle, steam_handle);

		if (data.bActive) {
			action_status.x = data.x;
			action_status.y = data.y;
		} else {
			action_status.x = 0.f;
			action_status.y = 0.f;
		}
	}
}

void ControllerMap::set_action_set(ActionSet set) {
	auto handle = action_sets.at(set);
	if (handle != SteamInput()->GetCurrentActionSet(controller_handle)) { std::cout << "Changing to set " << handle << std::endl; }
	SteamInput()->ActivateActionSet(STEAM_INPUT_HANDLE_ALL_CONTROLLERS, handle);
}

[[nodiscard]] auto ControllerMap::digital_action_name(DigitalAction action) const -> std::string_view { return SteamInput()->GetStringForDigitalActionName(digital_actions.at(action).steam_handle); }

void ControllerMap::handle_gamepad_connection(SteamInputDeviceConnected_t* data) {
	std::cout << "Connected controller with handle = " << data->m_ulConnectedDeviceHandle << std::endl;
	controller_handle = data->m_ulConnectedDeviceHandle;
}

void ControllerMap::handle_gamepad_disconnection(SteamInputDeviceDisconnected_t* data) {
	std::cout << "Disconnected controller with handle = " << data->m_ulDisconnectedDeviceHandle << std::endl;
	controller_handle = data->m_ulDisconnectedDeviceHandle;
}

void ControllerMap::open_bindings_overlay() { SteamInput()->ShowBindingPanel(controller_handle); }

auto ControllerMap::key_to_string(sf::Keyboard::Key key) -> std::string_view {
	// XXX: Replace by switch
	std::unordered_map<sf::Keyboard::Key, std::string_view> map{{sf::Keyboard::A, "A"},			  {sf::Keyboard::B, "B"},
																{sf::Keyboard::C, "C"},			  {sf::Keyboard::D, "D"},
																{sf::Keyboard::E, "E"},			  {sf::Keyboard::F, "F"},
																{sf::Keyboard::G, "G"},			  {sf::Keyboard::H, "H"},
																{sf::Keyboard::I, "I"},			  {sf::Keyboard::J, "J"},
																{sf::Keyboard::K, "K"},			  {sf::Keyboard::L, "L"},
																{sf::Keyboard::M, "M"},			  {sf::Keyboard::N, "N"},
																{sf::Keyboard::O, "O"},			  {sf::Keyboard::P, "P"},
																{sf::Keyboard::Q, "Q"},			  {sf::Keyboard::R, "R"},
																{sf::Keyboard::S, "S"},			  {sf::Keyboard::T, "T"},
																{sf::Keyboard::U, "U"},			  {sf::Keyboard::V, "V"},
																{sf::Keyboard::W, "W"},			  {sf::Keyboard::X, "X"},
																{sf::Keyboard::Y, "Y"},			  {sf::Keyboard::Z, "Z"},
																{sf::Keyboard::LShift, "LShift"}, {sf::Keyboard::RShift, "RShift"},
																{sf::Keyboard::Left, "Left"},	  {sf::Keyboard::Right, "Right"},
																{sf::Keyboard::Up, "Up"},		  {sf::Keyboard::Down, "Down"},
																{sf::Keyboard::Period, "Period"}, {sf::Keyboard::Num1, "1"},
																{sf::Keyboard::Num2, "2"},		  {sf::Keyboard::Num3, "3"},
																{sf::Keyboard::Space, "Space"},	  {sf::Keyboard::LControl, "LControl"},
																{sf::Keyboard::Escape, "Esc"},	  {sf::Keyboard::Unknown, "None"}};

	return map.at(key);
}

auto ControllerMap::string_to_key(std::string_view string) -> sf::Keyboard::Key {
	std::unordered_map<std::string_view, sf::Keyboard::Key> map{{"A", sf::Keyboard::A},			  {"B", sf::Keyboard::B},
																{"C", sf::Keyboard::C},			  {"D", sf::Keyboard::D},
																{"E", sf::Keyboard::E},			  {"F", sf::Keyboard::F},
																{"G", sf::Keyboard::G},			  {"H", sf::Keyboard::H},
																{"I", sf::Keyboard::I},			  {"J", sf::Keyboard::J},
																{"K", sf::Keyboard::K},			  {"L", sf::Keyboard::L},
																{"M", sf::Keyboard::M},			  {"N", sf::Keyboard::N},
																{"O", sf::Keyboard::O},			  {"P", sf::Keyboard::P},
																{"Q", sf::Keyboard::Q},			  {"R", sf::Keyboard::R},
																{"S", sf::Keyboard::S},			  {"T", sf::Keyboard::T},
																{"U", sf::Keyboard::U},			  {"V", sf::Keyboard::V},
																{"W", sf::Keyboard::W},			  {"X", sf::Keyboard::X},
																{"Y", sf::Keyboard::Y},			  {"Z", sf::Keyboard::Z},
																{"LShift", sf::Keyboard::LShift}, {"RShift", sf::Keyboard::RShift},
																{"Left", sf::Keyboard::Left},	  {"Right", sf::Keyboard::Right},
																{"Up", sf::Keyboard::Up},		  {"Down", sf::Keyboard::Down},
																{"Period", sf::Keyboard::Period}, {"1", sf::Keyboard::Num1},
																{"2", sf::Keyboard::Num2},		  {"3", sf::Keyboard::Num3},
																{"Space", sf::Keyboard::Space},	  {"LControl", sf::Keyboard::LControl},
																{"Esc", sf::Keyboard::Escape},	  {"Enter", sf::Keyboard::Enter}};

	if (map.contains(string)) {
		return map.at(string);
	} else {
		return sf::Keyboard::Unknown;
	}
}

void ControllerMap::set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Key key) { digital_actions.at(action).primary_binding = key; }
void ControllerMap::set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Key key) { digital_actions.at(action).secondary_binding = key; }

auto ControllerMap::get_primary_keyboard_binding(DigitalAction action) -> sf::Keyboard::Key { return digital_actions.at(action).primary_binding; }
auto ControllerMap::get_secondary_keyboard_binding(DigitalAction action) -> sf::Keyboard::Key { return digital_actions.at(action).secondary_binding; }

} // namespace config