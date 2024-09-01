#include "ControllerMap.hpp"
#include <steam/isteaminput.h>
#include <steam/steam_api.h>
#include <iostream>
#include "../service/ServiceProvider.hpp"

InputHandle_t controller{}; // XXX move to ControllerMap

namespace config {

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {
	std::cout << "Initializing Steam Input" << std::endl;
	if (!SteamInput()->Init(true)) {
		std::cout << "Could not initialize Steam Input!" << std::endl;
	} else {
		std::cout << "Steam Input initialized" << std::endl;
	}
	SteamInput()->SetInputActionManifestFilePath("C:\\Program Files (x86)\\Steam\\controller_config\\steam_input_manifest.vdf");
	InputHandle_t connected_controllers[STEAM_INPUT_MAX_COUNT];
	SteamInput()->RunFrame();
	auto connected_controllers_count = SteamInput()->GetConnectedControllers(connected_controllers);
	std::cout << "Connected controller count: " << connected_controllers_count << std::endl;
	if (connected_controllers_count > 0) { controller = connected_controllers[0]; }
	SteamInput()->ActivateActionSet(controller, SteamInput()->GetActionSetHandle("MenuControls"));
	std::cout << "Current action set: " << SteamInput()->GetCurrentActionSet(controller) << std::endl;

	label_to_control.insert({"main_action", Control(Action::main_action)});
	label_to_control.insert({"secondary_action", Control(Action::secondary_action)});
	label_to_control.insert({"tertiary_action", Control(Action::tertiary_action)});
	label_to_control.insert({"inspect", Control(Action::inspect)});
	label_to_control.insert({"sprint", Control(Action::sprint)});
	label_to_control.insert({"shield", Control(Action::shield)});
	label_to_control.insert({"menu_toggle", Control(Action::menu_toggle)});
	label_to_control.insert({"menu_toggle_secondary", Control(Action::menu_toggle_secondary)});
	label_to_control.insert({"arms_switch_left", Control(Action::arms_switch_left)});
	label_to_control.insert({"arms_switch_right", Control(Action::arms_switch_right)});
	label_to_control.insert({"left", Control(Action::left)});
	label_to_control.insert({"right", Control(Action::right)});
	label_to_control.insert({"up", Control(Action::up)});
	label_to_control.insert({"down", Control(Action::down)});
	label_to_control.insert({"menu_forward", Control(Action::menu_forward)});
	label_to_control.insert({"menu_back", Control(Action::menu_back)});

	steam_input_data.insert({"main_action", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("menu_select"), .analog_handle = 0}});
	steam_input_data.insert({"up", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("menu_up"), .analog_handle = 0}});
	steam_input_data.insert({"left", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("menu_left"), .analog_handle = 0}});
	steam_input_data.insert({"right", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("menu_right"), .analog_handle = 0}});
	steam_input_data.insert({"down", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("menu_down"), .analog_handle = 0}});
	steam_input_data.insert({"sprint", SteamInputData{.type = SteamInputData::Type::Digital, .digital_handle = SteamInput()->GetDigitalActionHandle("sprint"), .analog_handle = 0}});

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
	for (auto const& [tag, action] : steam_input_data) {
		switch (action.type) {
		case SteamInputData::Type::Analog: break;
		case SteamInputData::Type::Digital:
			auto const data = SteamInput()->GetDigitalActionData(controller, action.digital_handle);
			auto& control = label_to_control.at(tag);
			if (data.bState) {
				if (!control.state.test(config::ActionState::held)) {
					std::cout << "Pressed " << tag << std::endl;
					control.state.set(config::ActionState::triggered);
				} else {
					control.state.reset(config::ActionState::triggered);
				}
				control.state.set(config::ActionState::held);
			} else {
				if (control.state.test(config::ActionState::held)) {
					control.state.set(config::ActionState::released);
				} else {
					control.state.reset(config::ActionState::released);
				}
				control.state.reset(config::ActionState::held);
			}
			break;
		}
	}
}

} // namespace config