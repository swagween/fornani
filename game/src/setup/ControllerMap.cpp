#include "fornani/setup/ControllerMap.hpp"
#include <steam/isteaminput.h>

#include <tracy/Tracy.hpp>

#include <iostream>
#include "fornani/service/ServiceProvider.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(error : 4061) // Missing enum variants in switch cases
#endif

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

	case DigitalAction::inventory_open_map:
	case DigitalAction::inventory_close: return ActionSet::Inventory;

	case DigitalAction::map_open_inventory:
	case DigitalAction::map_close: return ActionSet::Map;

	case DigitalAction::menu_left:
	case DigitalAction::menu_right:
	case DigitalAction::menu_up:
	case DigitalAction::menu_down:
	case DigitalAction::menu_select:
	case DigitalAction::menu_switch_right:
	case DigitalAction::menu_switch_left:
	case DigitalAction::menu_cancel: return ActionSet::Menu;

	case DigitalAction::COUNT:
	default: assert(false && "Invalid action set in get_action_set_from_action");
	}
	assert(false && "Invalid action set in get_action_set_from_action");
	return ActionSet::Platformer; // This will never be hit
}

/// @brief Gets the action set that should also be active when the given action set is active.
/// @returns Whether `set` has a parent or not.
bool parent_action_set(ActionSet set, ActionSet* parent) {
	switch (set) {
	case ActionSet::Platformer: return false;

	case ActionSet::Inventory:
	case ActionSet::Map: *parent = ActionSet::Menu; return true;

	case ActionSet::Menu: return false;
	}
	return false;
}

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {
	std::cout << "Initializing Steam Input" << std::endl;
	if (!SteamInput()->Init(true)) {
		std::cout << "Could not initialize Steam Input!" << std::endl;
	} else {
		std::cout << "Steam Input initialized" << std::endl;
	}
	// TODO When we have a proper Steam App ID assigned, upload the steam input manifest into the game's depot.
	std::string input_action_manifest_path = svc.finder.resource_path() + "\\text\\input\\steam_input_manifest.vdf";
	if (!SteamInput()->SetInputActionManifestFilePath(input_action_manifest_path.c_str())) {
		// uh oh
		std::cout << "Could not set Action Manifest file path!" << std::endl;
		std::cout << "Path: " << input_action_manifest_path << std::endl;
	}
	SteamInput()->EnableDeviceCallbacks();

	SteamInput()->RunFrame();
	setup_action_handles();

	for (auto const& [action_id, action_name] : svc.data.action_names.object_view()) {
		auto action = get_action_by_identifier(action_id);
		digital_action_names[action] = action_name.as_string();
	}
	platformer_action_set = SteamInput()->GetActionSetHandle("Platformer");
	menu_action_set = SteamInput()->GetActionSetHandle("Menu");
	inventory_action_layer = SteamInput()->GetActionSetHandle("Menu_Inventory");
	map_action_layer = SteamInput()->GetActionSetHandle("Menu_Map");
}

void ControllerMap::setup_action_handles() {
#define XSTR(a) STR(a)
#define STR(a) #a
#define DEFINE_ACTION(action_name)                                                                                                                                                                                                             \
	digital_actions.insert({DigitalAction::action_name, {SteamInput()->GetDigitalActionHandle(XSTR(action_name)), DigitalActionStatus(DigitalAction::action_name), sf::Keyboard::Scancode::Unknown, sf::Keyboard::Scancode::Unknown}});

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
	DEFINE_ACTION(inventory_open_map);
	DEFINE_ACTION(inventory_close);

	// Map controls
	DEFINE_ACTION(map_open_inventory);
	DEFINE_ACTION(map_close);

	// Menu controls
	DEFINE_ACTION(menu_left);
	DEFINE_ACTION(menu_right);
	DEFINE_ACTION(menu_up);
	DEFINE_ACTION(menu_down);
	DEFINE_ACTION(menu_select);
	DEFINE_ACTION(menu_switch_left);
	DEFINE_ACTION(menu_switch_right);
	DEFINE_ACTION(menu_cancel);

	// Analog actions
	analog_actions.insert({AnalogAction::map_movement, {SteamInput()->GetAnalogActionHandle("map_movement"), AnalogActionStatus(AnalogAction::map_movement)}});

#undef DEFINE_ACTION
#undef STR
#undef XSTR
}

void ControllerMap::handle_event(std::optional<sf::Event> const event) {
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		if (last_controller_ty_used != ControllerType::keyboard) { reset_digital_action_states(); }
		last_controller_ty_used = ControllerType::keyboard;
		if (key_pressed->scancode != sf::Keyboard::Scancode::Unknown) { keys_pressed.insert(key_pressed->scancode); }
	} else if (auto const* key_released = event->getIf<sf::Event::KeyReleased>()) {
		keys_pressed.erase(key_released->scancode);
	}
}

void ControllerMap::update() {
	ZoneScopedN("ControllerMap::update");
	SteamInput()->RunFrame();
	m_actions_queried_this_update.clear();

	// SteamInput()->ActivateActionSet(STEAM_INPUT_HANDLE_ALL_CONTROLLERS, SteamInput()->GetActionSetHandle("MenuControls"));
	std::unordered_set<EInputActionOrigin> buttons_pressed_this_tick{};
	for (auto& [action, data] : digital_actions) {
		auto& [steam_handle, action_status, primary_key, secondary_key, was_active_last_tick] = data;

		bool pressed_on_gamepad = false;
		if (controller_handle && gamepad_input_enabled) {
			auto const data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
			pressed_on_gamepad = data.bState;
		}
		auto pressed_on_keyboard = keys_pressed.contains(primary_key) || keys_pressed.contains(secondary_key);

		if (pressed_on_gamepad) {
			if (last_controller_ty_used != ControllerType::gamepad) { reset_digital_action_states(); }
			last_controller_ty_used = ControllerType::gamepad;
		}
		if (pressed_on_keyboard) {
			if (last_controller_ty_used != ControllerType::keyboard) { reset_digital_action_states(); }
			last_controller_ty_used = ControllerType::keyboard;
		}

		// Determine if this action is active (bound on the current action set).
		bool active{};
		if (last_controller_ty_used == ControllerType::gamepad) {
			// If we are using a gamepad, use steam's bActive member, otherwise it freaks out (causes multiple inputs, sometimes fails to input, etc)
			auto const data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
			active = data.bActive;
		} else {
			// Otherwise we just check the current action set manually.
			config::ActionSet action_set = get_action_set_from_action(action);
			config::ActionSet parent_of_active_set{};
			if (parent_action_set(active_action_set, &parent_of_active_set)) {
				active = active_action_set == action_set || action_set == parent_of_active_set;
			} else {
				active = active_action_set == action_set;
			}
		}
		auto triggered = pressed_on_gamepad || pressed_on_keyboard;

		if (triggered && active) {
			action_status.released = false;
			// Avoid actions being immediately triggered when switching action sets
			if (!action_status.held && was_active_last_tick) {
				// std::cout << "Pressed " << SteamInput()->GetStringForDigitalActionName(steam_handle) << std::endl;
				action_status.triggered = true;
			} else {
				action_status.triggered = false;
			}
			action_status.held = true;
		} else {
			action_status.triggered = false;
			if (action_status.held) {
				// std::cout << "Released " << SteamInput()->GetStringForDigitalActionName(steam_handle) << std::endl;
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

[[nodiscard]] auto ControllerMap::digital_action_status(DigitalAction action) -> DigitalActionStatus {
	m_actions_queried_this_update.insert(action);

	return digital_actions.at(action).status;
}

[[nodiscard]] auto ControllerMap::digital_action_name(DigitalAction action) const -> std::string_view { return digital_action_names.at(action); }

[[nodiscard]] auto ControllerMap::digital_action_source(DigitalAction action) const -> DigitalActionSource {
	auto controller_origin = k_EInputActionOrigin_None;
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
			if (origins[0] < k_EInputActionOrigin_Count) { controller_origin = origins[0]; }
		}
	}
	return DigitalActionSource{.controller_origin = controller_origin, .key = get_primary_keyboard_binding(action)};
}

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
	last_controller_ty_used = ControllerType::gamepad; // Quickly switch to gamepad input
	setup_action_handles();
	set_action_set(active_action_set);
}

void ControllerMap::handle_gamepad_disconnection(SteamInputDeviceDisconnected_t* data) {
	std::cout << "Disconnected controller with handle = " << data->m_ulDisconnectedDeviceHandle << std::endl;
	if (controller_handle != 0) { out.gamepad_disconnected = true; }
	controller_handle = 0;
	last_controller_ty_used = ControllerType::keyboard; // Quickly switch to keyboard input
}

void ControllerMap::open_bindings_overlay() const {
	if (!gamepad_connected()) { return; }
	SteamInput()->ShowBindingPanel(controller_handle);
}
auto ControllerMap::key_to_string(sf::Keyboard::Scancode key) const -> std::string_view {
	static std::unordered_map<sf::Keyboard::Scancode, std::string_view> map{{sf::Keyboard::Scancode::A, "A"},			 {sf::Keyboard::Scancode::B, "B"},
																	   {sf::Keyboard::Scancode::C, "C"},			 {sf::Keyboard::Scancode::D, "D"},
																	   {sf::Keyboard::Scancode::E, "E"},			 {sf::Keyboard::Scancode::F, "F"},
																	   {sf::Keyboard::Scancode::G, "G"},			 {sf::Keyboard::Scancode::H, "H"},
																	   {sf::Keyboard::Scancode::I, "I"},			 {sf::Keyboard::Scancode::J, "J"},
																	   {sf::Keyboard::Scancode::K, "K"},			 {sf::Keyboard::Scancode::L, "L"},
																	   {sf::Keyboard::Scancode::M, "M"},			 {sf::Keyboard::Scancode::N, "N"},
																	   {sf::Keyboard::Scancode::O, "O"},			 {sf::Keyboard::Scancode::P, "P"},
																	   {sf::Keyboard::Scancode::Q, "Q"},			 {sf::Keyboard::Scancode::R, "R"},
																	   {sf::Keyboard::Scancode::S, "S"},			 {sf::Keyboard::Scancode::T, "T"},
																	   {sf::Keyboard::Scancode::U, "U"},			 {sf::Keyboard::Scancode::V, "V"},
																	   {sf::Keyboard::Scancode::W, "W"},			 {sf::Keyboard::Scancode::X, "X"},
																	   {sf::Keyboard::Scancode::Y, "Y"},			 {sf::Keyboard::Scancode::Z, "Z"},
																	   {sf::Keyboard::Scancode::LShift, "LShift"}, {sf::Keyboard::Scancode::RShift, "RShift"},
																	   {sf::Keyboard::Scancode::Left, "Left"},	 {sf::Keyboard::Scancode::Right, "Right"},
																	   {sf::Keyboard::Scancode::Up, "Up"},		 {sf::Keyboard::Scancode::Down, "Down"},
																	   {sf::Keyboard::Scancode::Period, "Period"}, {sf::Keyboard::Scancode::Num1, "1"},
																	   {sf::Keyboard::Scancode::Num2, "2"},		 {sf::Keyboard::Scancode::Num3, "3"},
																	   {sf::Keyboard::Scancode::Space, "Space"},	 {sf::Keyboard::Scancode::LControl, "LControl"},
																	   {sf::Keyboard::Scancode::Escape, "Esc"},	 {sf::Keyboard::Scancode::Unknown, "None"}};

	return map.at(key);
}

auto ControllerMap::string_to_key(std::string_view string) const -> sf::Keyboard::Scancode {
	static std::unordered_map<std::string_view, sf::Keyboard::Scancode> map{{"A", sf::Keyboard::Scancode::A},			{"B", sf::Keyboard::Scancode::B},
																			{"C", sf::Keyboard::Scancode::C},			{"D", sf::Keyboard::Scancode::D},
																			{"E", sf::Keyboard::Scancode::E},			{"F", sf::Keyboard::Scancode::F},
																			{"G", sf::Keyboard::Scancode::G},			{"H", sf::Keyboard::Scancode::H},
																			{"I", sf::Keyboard::Scancode::I},			{"J", sf::Keyboard::Scancode::J},
																			{"K", sf::Keyboard::Scancode::K},			{"L", sf::Keyboard::Scancode::L},
																			{"M", sf::Keyboard::Scancode::M},			{"N", sf::Keyboard::Scancode::N},
																			{"O", sf::Keyboard::Scancode::O},			{"P", sf::Keyboard::Scancode::P},
																			{"Q", sf::Keyboard::Scancode::Q},			{"R", sf::Keyboard::Scancode::R},
																			{"S", sf::Keyboard::Scancode::S},			{"T", sf::Keyboard::Scancode::T},
																			{"U", sf::Keyboard::Scancode::U},			{"V", sf::Keyboard::Scancode::V},
																			{"W", sf::Keyboard::Scancode::W},			{"X", sf::Keyboard::Scancode::X},
																			{"Y", sf::Keyboard::Scancode::Y},			{"Z", sf::Keyboard::Scancode::Z},
																			{"LShift", sf::Keyboard::Scancode::LShift}, {"RShift", sf::Keyboard::Scancode::RShift},
																			{"Left", sf::Keyboard::Scancode::Left},		{"Right", sf::Keyboard::Scancode::Right},
																			{"Up", sf::Keyboard::Scancode::Up},			{"Down", sf::Keyboard::Scancode::Down},
																			{"Period", sf::Keyboard::Scancode::Period}, {"1", sf::Keyboard::Scancode::Num1},
																			{"2", sf::Keyboard::Scancode::Num2},		{"3", sf::Keyboard::Scancode::Num3},
																			{"Space", sf::Keyboard::Scancode::Space},	{"LControl", sf::Keyboard::Scancode::LControl},
																			{"Esc", sf::Keyboard::Scancode::Escape},	{"Enter", sf::Keyboard::Scancode::Enter}};

	if (map.contains(string)) {
		return map.at(string);
	} else {
		return sf::Keyboard::Scancode::Unknown;
	}
}

bool ControllerMap::process_gamepad_disconnection() {
	auto ret = out.gamepad_disconnected;
	out.gamepad_disconnected = false;
	return ret;
}

void ControllerMap::set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) { digital_actions.at(action).primary_binding = key; }
void ControllerMap::set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) { digital_actions.at(action).secondary_binding = key; }

auto ControllerMap::get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return digital_actions.at(action).primary_binding; }
auto ControllerMap::get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return digital_actions.at(action).secondary_binding; }

void ControllerMap::reset_digital_action_states() {
	for (auto& [action, state] : digital_actions) {
		state.status = DigitalActionStatus(action);
		state.was_active_last_tick = false;
	}
}

auto ControllerMap::get_action_by_identifier(std::string_view id) -> config::DigitalAction {
	static std::unordered_map<std::string_view, config::DigitalAction> const map = {
		{"platformer_left", config::DigitalAction::platformer_left},
		{"platformer_right", config::DigitalAction::platformer_right},
		{"platformer_up", config::DigitalAction::platformer_up},
		{"platformer_down", config::DigitalAction::platformer_down},
		{"platformer_jump", config::DigitalAction::platformer_jump},
		{"platformer_shoot", config::DigitalAction::platformer_shoot},
		{"platformer_sprint", config::DigitalAction::platformer_sprint},
		{"platformer_shield", config::DigitalAction::platformer_shield},
		{"platformer_inspect", config::DigitalAction::platformer_inspect},
		{"platformer_arms_switch_left", config::DigitalAction::platformer_arms_switch_left},
		{"platformer_arms_switch_right", config::DigitalAction::platformer_arms_switch_right},
		{"platformer_open_inventory", config::DigitalAction::platformer_open_inventory},
		{"platformer_open_map", config::DigitalAction::platformer_open_map},
		{"platformer_toggle_pause", config::DigitalAction::platformer_toggle_pause},
		{"inventory_open_map", config::DigitalAction::inventory_open_map},
		{"inventory_close", config::DigitalAction::inventory_close},
		{"map_open_inventory", config::DigitalAction::map_open_inventory},
		{"map_close", config::DigitalAction::map_close},
		{"menu_left", config::DigitalAction::menu_left},
		{"menu_right", config::DigitalAction::menu_right},
		{"menu_up", config::DigitalAction::menu_up},
		{"menu_down", config::DigitalAction::menu_down},
		{"menu_select", config::DigitalAction::menu_select},
		{"menu_cancel", config::DigitalAction::menu_cancel},
		{"menu_switch_left", config::DigitalAction::menu_switch_left},
		{"menu_switch_right", config::DigitalAction::menu_switch_right},
	};

	return map.at(id);
}

} // namespace config

#ifdef _MSC_VER
#pragma warning(pop)
#endif
