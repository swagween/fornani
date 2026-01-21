
#include <steam/isteaminput.h>
#include <ccmath/ext/clamp.hpp>
#include <fornani/gui/ActionControlIconQuery.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/ControllerMap.hpp>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(error : 4061) // Missing enum variants in switch cases
#endif

namespace fornani::config {

auto get_action_set_from_action(DigitalAction action) -> ActionSet {
	switch (action) {
	case DigitalAction::platformer_left:
	case DigitalAction::platformer_right:
	case DigitalAction::platformer_up:
	case DigitalAction::platformer_down:
	case DigitalAction::platformer_jump:
	case DigitalAction::platformer_shoot:
	case DigitalAction::platformer_sprint:
	case DigitalAction::platformer_slide:
	case DigitalAction::platformer_dash:
	case DigitalAction::platformer_inspect:
	case DigitalAction::platformer_arms_switch_left:
	case DigitalAction::platformer_arms_switch_right:
	case DigitalAction::platformer_open_inventory:
	case DigitalAction::platformer_toggle_pause: return ActionSet::Platformer;

	case DigitalAction::inventory_close: return ActionSet::Inventory;

	case DigitalAction::menu_left:
	case DigitalAction::menu_right:
	case DigitalAction::menu_up:
	case DigitalAction::menu_down:
	case DigitalAction::menu_select:
	case DigitalAction::menu_tab_right:
	case DigitalAction::menu_tab_left:
	case DigitalAction::menu_cancel:
	case DigitalAction::menu_confirm: return ActionSet::Menu;

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

ControllerMap::ControllerMap(automa::ServiceProvider& svc) : m_stick_sensitivity{0.2f} {
	NANI_LOG_INFO(m_logger, "Initializing Steam Input");
	if (!SteamInput()->Init(true)) {
		NANI_LOG_WARN(m_logger, "Could not initialize Steam Input!");
	} else {
		NANI_LOG_INFO(m_logger, "Steam Input initialized");
	}
	// TODO When we have a proper Steam App ID assigned, upload the steam input manifest into the game's depot.
	std::string input_action_manifest_path = svc.finder.resource_path() + "\\text\\input\\steam_input_manifest.vdf";
	if (!SteamInput()->SetInputActionManifestFilePath(input_action_manifest_path.c_str())) {
		// uh oh
		NANI_LOG_ERROR(m_logger, "Could not set Action Manifest file path!");
		NANI_LOG_ERROR(m_logger, "Path: {}", input_action_manifest_path);
	}
	SteamInput()->EnableDeviceCallbacks();

	SteamInput()->RunFrame();
	setup_action_handles();

	for (auto const& [action_id, action_name] : svc.data.action_names.as_object()) {
		auto action = get_action_by_identifier(action_id);
		digital_action_names[action] = action_name.as_string();
	}
	platformer_action_set = SteamInput()->GetActionSetHandle("Platformer");
	menu_action_set = SteamInput()->GetActionSetHandle("Menu");
	inventory_action_layer = SteamInput()->GetActionSetHandle("Menu_Inventory");
	map_action_layer = SteamInput()->GetActionSetHandle("Menu_Map");
	active_action_set = ActionSet::Platformer;
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
	DEFINE_ACTION(platformer_slide);
	DEFINE_ACTION(platformer_dash);
	DEFINE_ACTION(platformer_inspect);
	DEFINE_ACTION(platformer_arms_switch_left);
	DEFINE_ACTION(platformer_arms_switch_right);
	DEFINE_ACTION(platformer_open_inventory);
	DEFINE_ACTION(platformer_toggle_pause);

	// Inventory controls
	DEFINE_ACTION(inventory_close);

	// Menu controls
	DEFINE_ACTION(menu_left);
	DEFINE_ACTION(menu_right);
	DEFINE_ACTION(menu_up);
	DEFINE_ACTION(menu_down);
	DEFINE_ACTION(menu_select);
	DEFINE_ACTION(menu_tab_left);
	DEFINE_ACTION(menu_tab_right);
	DEFINE_ACTION(menu_cancel);
	DEFINE_ACTION(menu_confirm);

	// Analog actions
	analog_actions.insert({AnalogAction::map_movement, {SteamInput()->GetAnalogActionHandle("map_movement"), AnalogActionStatus(AnalogAction::map_movement)}});
	analog_actions.insert({AnalogAction::platformer_movement, {SteamInput()->GetAnalogActionHandle("platformer_movement"), AnalogActionStatus(AnalogAction::platformer_movement)}});

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

void ControllerMap::flush_inputs() {
	for (auto& [action, data] : digital_actions) {
		auto& [steam_handle, action_status, primary_key, secondary_key, was_active_last_tick] = data;
		action_status.triggered = false;
	}
}

sf::Vector2i ControllerMap::get_icon_lookup_by_action(DigitalAction action) const {
	auto source = digital_action_source(action);
	return (source.controller_origin == k_EInputActionOrigin_None) ? gui::get_key_coordinates(source.key) : gui::get_controller_button_coordinates(source.controller_origin);
}

void ControllerMap::update() {
	SteamInput()->RunFrame();
	m_actions_queried_this_update.clear();
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
				auto data = SteamInput()->GetDigitalActionData(controller_handle, steam_handle);
				// NANI_LOG_DEBUG(m_logger, "Pressed {}", SteamInput()->GetStringForDigitalActionName(steam_handle));
				action_status.triggered = true;
			} else {
				action_status.triggered = false;
			}
			action_status.held = true;
		} else {
			action_status.triggered = false;
			if (action_status.held) {
				// NANI_LOG_DEBUG(m_logger, "Released {}", SteamInput()->GetStringForDigitalActionName(steam_handle));
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

		if (data.bActive) { NANI_LOG_DEBUG(m_logger, "Analog action data: [ {}, {} ]", data.x, data.y); }

		if (data.bActive) {
			action_status.x = data.x;
			action_status.y = data.y;
		} else {
			action_status.x = 0.f;
			action_status.y = 0.f;
		}
	}

	// if (last_controller_type_used() != ControllerType::gamepad) { m_joystick_throttle = {}; }
}

void ControllerMap::set_action_set(ActionSet set) {
	if (controller_handle && set != active_action_set) {
		NANI_LOG_DEBUG(m_logger, "Set action set to {}", static_cast<int>(set));
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

[[nodiscard]] auto ControllerMap::digital_action_name(DigitalAction action) const -> std::string_view {
	if (digital_action_names.contains(action)) {
		return digital_action_names.at(action);
	} else {
		NANI_LOG_ERROR(m_logger, "Action missing from action_names.json");
		return std::string_view{};
	}
}

[[nodiscard]] auto ControllerMap::digital_action_source(DigitalAction action) const -> DigitalActionSource {
	auto controller_origin = k_EInputActionOrigin_None;
	if (controller_handle) {
		auto action_set = get_action_set_from_action(action);
		InputActionSetHandle_t handle{};
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
		InputActionSetHandle_t handle{};
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
	NANI_LOG_INFO(m_logger, "Connected controller with handle [{}]", data->m_ulConnectedDeviceHandle);
	controller_handle = data->m_ulConnectedDeviceHandle;
	last_controller_ty_used = ControllerType::gamepad; // Quickly switch to gamepad input
	setup_action_handles();
	set_action_set(active_action_set);
}

void ControllerMap::handle_gamepad_disconnection(SteamInputDeviceDisconnected_t* data) {
	NANI_LOG_INFO(m_logger, "Disconnected controller with handle [{}] ", data->m_ulDisconnectedDeviceHandle);
	if (controller_handle != 0) { out.gamepad_disconnected = true; }
	controller_handle = 0;
	last_controller_ty_used = ControllerType::keyboard; // Quickly switch to keyboard input
}

void ControllerMap::open_bindings_overlay() const {
	if (!gamepad_connected()) { return; }
	SteamInput()->ShowBindingPanel(controller_handle);
}
auto ControllerMap::key_to_string(sf::Keyboard::Scancode key) const -> std::string_view {
	static std::unordered_map<sf::Keyboard::Scancode, std::string_view> map{{sf::Keyboard::Scancode::A, "A"},			{sf::Keyboard::Scancode::B, "B"},
																			{sf::Keyboard::Scancode::C, "C"},			{sf::Keyboard::Scancode::D, "D"},
																			{sf::Keyboard::Scancode::E, "E"},			{sf::Keyboard::Scancode::F, "F"},
																			{sf::Keyboard::Scancode::G, "G"},			{sf::Keyboard::Scancode::H, "H"},
																			{sf::Keyboard::Scancode::I, "I"},			{sf::Keyboard::Scancode::J, "J"},
																			{sf::Keyboard::Scancode::K, "K"},			{sf::Keyboard::Scancode::L, "L"},
																			{sf::Keyboard::Scancode::M, "M"},			{sf::Keyboard::Scancode::N, "N"},
																			{sf::Keyboard::Scancode::O, "O"},			{sf::Keyboard::Scancode::P, "P"},
																			{sf::Keyboard::Scancode::Q, "Q"},			{sf::Keyboard::Scancode::R, "R"},
																			{sf::Keyboard::Scancode::S, "S"},			{sf::Keyboard::Scancode::T, "T"},
																			{sf::Keyboard::Scancode::U, "U"},			{sf::Keyboard::Scancode::V, "V"},
																			{sf::Keyboard::Scancode::W, "W"},			{sf::Keyboard::Scancode::X, "X"},
																			{sf::Keyboard::Scancode::Y, "Y"},			{sf::Keyboard::Scancode::Z, "Z"},
																			{sf::Keyboard::Scancode::LShift, "LShift"}, {sf::Keyboard::Scancode::RShift, "RShift"},
																			{sf::Keyboard::Scancode::Left, "Left"},		{sf::Keyboard::Scancode::Right, "Right"},
																			{sf::Keyboard::Scancode::Up, "Up"},			{sf::Keyboard::Scancode::Down, "Down"},
																			{sf::Keyboard::Scancode::Period, "Period"}, {sf::Keyboard::Scancode::Num1, "1"},
																			{sf::Keyboard::Scancode::Num2, "2"},		{sf::Keyboard::Scancode::Num3, "3"},
																			{sf::Keyboard::Scancode::Space, "Space"},	{sf::Keyboard::Scancode::LControl, "LControl"},
																			{sf::Keyboard::Scancode::Enter, "Enter"},	{sf::Keyboard::Scancode::Escape, "Esc"},
																			{sf::Keyboard::Scancode::Unknown, "None"}};

	return map.contains(key) ? map.at(key) : "<Unknown>";
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

	return map.contains(string) ? map.at(string) : sf::Keyboard::Scancode::Unknown;
}

auto config::ControllerMap::has_forbidden_duplicate_binding() const -> bool {
	for (auto& binding : digital_actions) {
		for (auto& other : digital_actions) {
			if (get_action_set_from_action(binding.first) != get_action_set_from_action(other.first)) { continue; }
			if (other.first != binding.first && binding.second.primary_binding == other.second.primary_binding) {
				if (binding.first == DigitalAction::menu_select) { return true; }
				if (binding.first == DigitalAction::menu_confirm) { return true; }
				if (binding.first == DigitalAction::menu_up) { return true; }
				if (binding.first == DigitalAction::menu_down) { return true; }
				if (binding.first == DigitalAction::menu_left) { return true; }
				if (binding.first == DigitalAction::menu_right) { return true; }
			}
		}
	}
	return false;
}

auto ControllerMap::is_bound_to_same_input(DigitalAction first, DigitalAction second) const -> bool { return digital_actions.at(first).primary_binding == digital_actions.at(second).primary_binding; }

bool ControllerMap::process_gamepad_disconnection() {
	auto ret = out.gamepad_disconnected;
	out.gamepad_disconnected = false;
	return ret;
}

void ControllerMap::set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (digital_actions.contains(action)) { digital_actions.at(action).primary_binding = key; }
}
void ControllerMap::set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (digital_actions.contains(action)) { digital_actions.at(action).secondary_binding = key; }
}

auto ControllerMap::get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return digital_actions.at(action).primary_binding; }
auto ControllerMap::get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return digital_actions.at(action).secondary_binding; }

void ControllerMap::reset_digital_action_states() {
	for (auto& [action, state] : digital_actions) {
		state.status = DigitalActionStatus(action);
		state.was_active_last_tick = false;
	}
}

auto ControllerMap::get_action_by_identifier(std::string_view id) -> config::DigitalAction {
	static std::unordered_map<std::string_view, config::DigitalAction> const map = {{"platformer_left", config::DigitalAction::platformer_left},
																					{"platformer_right", config::DigitalAction::platformer_right},
																					{"platformer_up", config::DigitalAction::platformer_up},
																					{"platformer_down", config::DigitalAction::platformer_down},
																					{"platformer_jump", config::DigitalAction::platformer_jump},
																					{"platformer_shoot", config::DigitalAction::platformer_shoot},
																					{"platformer_sprint", config::DigitalAction::platformer_sprint},
																					{"platformer_slide", config::DigitalAction::platformer_slide},
																					{"platformer_dash", config::DigitalAction::platformer_dash},
																					{"platformer_inspect", config::DigitalAction::platformer_inspect},
																					{"platformer_arms_switch_left", config::DigitalAction::platformer_arms_switch_left},
																					{"platformer_arms_switch_right", config::DigitalAction::platformer_arms_switch_right},
																					{"platformer_open_inventory", config::DigitalAction::platformer_open_inventory},
																					{"platformer_toggle_pause", config::DigitalAction::platformer_toggle_pause},
																					{"inventory_close", config::DigitalAction::inventory_close},
																					{"menu_left", config::DigitalAction::menu_left},
																					{"menu_right", config::DigitalAction::menu_right},
																					{"menu_up", config::DigitalAction::menu_up},
																					{"menu_down", config::DigitalAction::menu_down},
																					{"menu_select", config::DigitalAction::menu_select},
																					{"menu_cancel", config::DigitalAction::menu_cancel},
																					{"menu_tab_left", config::DigitalAction::menu_tab_left},
																					{"menu_tab_right", config::DigitalAction::menu_tab_right},
																					{"menu_confirm", config::DigitalAction::menu_confirm}};

	return map.contains(id) ? map.at(id) : config::DigitalAction::COUNT;
}

auto config::ControllerMap::get_joystick_throttle() const -> sf::Vector2f {
	if (!gamepad_connected()) { return {}; }
	return m_joystick_throttle;
}

auto ControllerMap::get_i_joystick_throttle(bool exclusive) const -> sf::Vector2i {
	if (!gamepad_connected()) { return {}; }
	auto ret = sf::Vector2i{static_cast<int>(std::ceil(m_joystick_throttle.x)), static_cast<int>(std::ceil(m_joystick_throttle.y))};
	if (exclusive) {
		if (abs(m_joystick_throttle.x) > abs(m_joystick_throttle.y)) { ret.y = 0.f; }
		if (abs(m_joystick_throttle.x) <= abs(m_joystick_throttle.y)) { ret.x = 0.f; }
	}
	return ret;
}

void ControllerMap::set_joystick_throttle(sf::Vector2f throttle) {
	// constrict throttle based on stick sensitivity
	m_joystick_throttle.x = ccm::ext::clamp(throttle.x / 100.f, -1.f, 1.f);
	m_joystick_throttle.y = ccm::ext::clamp(throttle.y / 100.f, -1.f, 1.f);
	if (ccm::abs(m_joystick_throttle.x) < m_stick_sensitivity) { m_joystick_throttle.x = 0.f; }
	if (ccm::abs(m_joystick_throttle.y) < m_stick_sensitivity) { m_joystick_throttle.y = 0.f; }
}

void ControllerMap::set_last_key_pressed(sf::Keyboard::Scancode to_key) { m_last_key_pressed = to_key; }

void config::ControllerMap::set_keyboard_input_detected(bool flag) { m_keyboard_input_detected = flag; }

} // namespace fornani::config

#ifdef _MSC_VER
#pragma warning(pop)
#endif
