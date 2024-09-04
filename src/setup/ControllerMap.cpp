#include "ControllerMap.hpp"
#include <steam/isteaminput.h>
#include <iostream>
#include "../service/ServiceProvider.hpp"

namespace config {

auto get_action_set_from_action(DigitalAction action) -> ActionSet {
	switch (action) {
	case DigitalAction::platformer_left:
	case DigitalAction::platformer_right:
	case DigitalAction::platformer_up:
	case DigitalAction::platformer_down:
	case DigitalAction::platformer_jump:
	case DigitalAction::platformer_shoot:
	case DigitalAction::platformer_sprint:
	case DigitalAction::platformer_shield:
	case DigitalAction::platformer_inspect:
	case DigitalAction::platformer_arms_switch_left:
	case DigitalAction::platformer_arms_switch_right:
	case DigitalAction::platformer_open_inventory:
	case DigitalAction::platformer_open_map:
	case DigitalAction::platformer_toggle_pause: return ActionSet::Platformer;

	case DigitalAction::inventory_close: return ActionSet::Inventory;

	case DigitalAction::map_close: return ActionSet::Map;

	case DigitalAction::menu_left:
	case DigitalAction::menu_right:
	case DigitalAction::menu_up:
	case DigitalAction::menu_down:
	case DigitalAction::menu_select:
	case DigitalAction::menu_cancel: return ActionSet::Menu;
	}
}

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {
	std::cout << "Initializing Steam Input" << std::endl;
	if (!SteamInput()->Init(true)) {
		std::cout << "Could not initialize Steam Input!" << std::endl;
	} else {
		std::cout << "Steam Input initialized" << std::endl;
	}
	// XXX Bundle XBox/PS/Steam Deck action manifests with game (Or at least just XBox for now)
	// SteamInput()->SetInputActionManifestFilePath("C:\\Program Files (x86)\\Steam\\controller_config\\steam_input_manifest.vdf");
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
	DEFINE_ACTION(inventory_close);

	// Map controls
	DEFINE_ACTION(map_close);

	// Menu controls
	DEFINE_ACTION(menu_left);
	DEFINE_ACTION(menu_right);
	DEFINE_ACTION(menu_up);
	DEFINE_ACTION(menu_down);
	DEFINE_ACTION(menu_select);
	DEFINE_ACTION(menu_cancel);

	// Analog actions
	analog_actions.insert({AnalogAction::map_movement, {SteamInput()->GetAnalogActionHandle("map_movement"), AnalogActionStatus(AnalogAction::map_movement)}});

#undef DEFINE_ACTION
#undef STR
#undef XSTR

	platformer_action_set = SteamInput()->GetActionSetHandle("Platformer");
	menu_action_set = SteamInput()->GetActionSetHandle("Menu");
	inventory_action_layer = SteamInput()->GetActionSetHandle("Menu_Inventory");
	map_action_layer = SteamInput()->GetActionSetHandle("Menu_Map");

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

void ControllerMap::update(bool has_focus) {
	SteamInput()->RunFrame();
	// SteamInput()->ActivateActionSet(STEAM_INPUT_HANDLE_ALL_CONTROLLERS, SteamInput()->GetActionSetHandle("MenuControls"));
	std::unordered_set<EInputActionOrigin> buttons_pressed_this_tick{};
	for (auto& [action, data] : digital_actions) {
		auto& [steam_handle, action_status, primary_key, secondary_key, was_active_last_tick] = data;

		bool pressed_on_gamepad = false;
		if (controller_handle) {
			auto const data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
			pressed_on_gamepad = data.bActive && data.bState;
		}
		auto pressed_on_keyboard = sf::Keyboard::isKeyPressed(primary_key) || sf::Keyboard::isKeyPressed(secondary_key);
		auto triggered = has_focus && (pressed_on_gamepad || pressed_on_keyboard);
		config::ActionSet action_set = get_action_set_from_action(action);
		bool active = action_set == active_action_set || ((action_set == config::ActionSet::Inventory || action_set == config::ActionSet::Map) && active_action_set == config::ActionSet::Menu);
		if (triggered && active) {
			action_status.released = false;
			// Avoid actions being inmediately triggered when switching action sets
			if (!action_status.held && was_active_last_tick) {
				std::cout << "Pressed " << SteamInput()->GetStringForDigitalActionName(steam_handle) << std::endl;
				action_status.triggered = true;
			} else {
				action_status.triggered = false;
			}
			action_status.held = true;
		} else {
			action_status.triggered = false;
			// Avoid releasing if just switching action sets
			if (action_status.held && active) {
				std::cout << "Released " << SteamInput()->GetStringForDigitalActionName(steam_handle) << std::endl;
				action_status.released = true;
			} else {
				action_status.released = false;
			}
			action_status.held = false;
		}

		was_active_last_tick = active;
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
	if (controller_handle && set != active_action_set) {
		std::cout << "Set action set to " << (int)set << std::endl;
		SteamInput()->DeactivateAllActionSetLayers(controller_handle);
		switch (set) {
		case ActionSet::Inventory:
			SteamInput()->ActivateActionSet(controller_handle, menu_action_set);
			SteamInput()->ActivateActionSetLayer(controller_handle, inventory_action_layer);
			break;
		case ActionSet::Map:
			SteamInput()->ActivateActionSet(controller_handle, menu_action_set);
			SteamInput()->ActivateActionSetLayer(controller_handle, map_action_layer);
			break;
		case ActionSet::Menu: SteamInput()->ActivateActionSet(controller_handle, menu_action_set); break;
		case ActionSet::Platformer: SteamInput()->ActivateActionSet(controller_handle, platformer_action_set); break;
		}
	}
	active_action_set = set;
}

[[nodiscard]] auto ControllerMap::digital_action_name(DigitalAction action) const -> std::string_view { return SteamInput()->GetStringForDigitalActionName(digital_actions.at(action).steam_handle); }

[[nodiscard]] auto ControllerMap::digital_action_source_name(DigitalAction action) const -> std::string_view {
	if (controller_handle) {
		auto action_set = get_action_set_from_action(action);
		InputActionSetHandle_t handle;
		switch (action_set) {
		case ActionSet::Inventory: handle = inventory_action_layer; break;
		case ActionSet::Map: handle = map_action_layer; break;
		case ActionSet::Menu: handle = menu_action_set; break;
		case ActionSet::Platformer: handle = platformer_action_set; break;
		}

		EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
		if (SteamInput()->GetDigitalActionOrigins(controller_handle, handle, digital_actions.at(action).steam_handle, origins) > 0) {
			return SteamInput()->GetStringForActionOrigin(origins[0]);
		} else {
			return "Unassigned";
		}
	} else {
		return key_to_string(digital_actions.at(action).primary_binding);
	}
}

void ControllerMap::handle_gamepad_connection(SteamInputDeviceConnected_t* data) {
	std::cout << "Connected controller with handle = " << data->m_ulConnectedDeviceHandle << std::endl;
	controller_handle = data->m_ulConnectedDeviceHandle;
}

void ControllerMap::handle_gamepad_disconnection(SteamInputDeviceDisconnected_t* data) {
	std::cout << "Disconnected controller with handle = " << data->m_ulDisconnectedDeviceHandle << std::endl;
	controller_handle = data->m_ulDisconnectedDeviceHandle;
}

void ControllerMap::open_bindings_overlay() const { SteamInput()->ShowBindingPanel(controller_handle); }

auto ControllerMap::key_to_string(sf::Keyboard::Key key) const -> std::string_view {
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

auto ControllerMap::string_to_key(std::string_view string) const -> sf::Keyboard::Key {
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

auto ControllerMap::get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Key { return digital_actions.at(action).primary_binding; }
auto ControllerMap::get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Key { return digital_actions.at(action).secondary_binding; }

} // namespace config