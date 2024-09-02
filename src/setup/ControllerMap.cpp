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
	SteamInput()->RunFrame();
	auto connected_controllers_count = SteamInput()->GetConnectedControllers(connected_controllers);
	std::cout << "Connected controller count: " << connected_controllers_count << std::endl;
	if (connected_controllers_count > 0) { controller_handle = connected_controllers[0]; }

#define XSTR(a) STR(a)
#define STR(a) #a
#define DEFINE_ACTION(action_name) digital_actions.insert({DigitalAction::action_name, {SteamInput()->GetDigitalActionHandle(XSTR(action_name)), DigitalActionStatus(DigitalAction::action_name)}})

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

	SteamInput()->ActivateActionSet(controller_handle, action_sets[ActionSet::Menu]);
	std::cout << "Current action set: " << SteamInput()->GetCurrentActionSet(controller_handle) << std::endl;

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
		auto& [steam_handle, action_status] = pair;

		auto const data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
		if (data.bActive && data.bState) {
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

[[nodiscard]] auto ControllerMap::digital_action_name(DigitalAction action) const -> std::string_view { return SteamInput()->GetStringForDigitalActionName(digital_actions.at(action).first); }

void ControllerMap::handle_gamepad_connection(SteamInputDeviceConnected_t* data) {
	std::cout << "Connected controller with handle = " << data->m_ulConnectedDeviceHandle << std::endl;
	controller_handle = data->m_ulConnectedDeviceHandle;
}

} // namespace config