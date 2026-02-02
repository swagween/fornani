
#include <djson/json.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani::input {

constexpr auto default_joystick_sensitivity_v = 0.2f;

InputSystem::InputSystem(ResourceFinder& finder) : m_stick_sensitivity{default_joystick_sensitivity_v} {
	NANI_LOG_INFO(m_logger, "Initializing Steam Input");
	if (!SteamInput()->Init(true)) {
		NANI_LOG_WARN(m_logger, "Could not initialize Steam Input!");
	} else {
		NANI_LOG_INFO(m_logger, "Steam Input initialized");
	}
	std::string input_action_manifest_path = finder.resource_path() + "\\text\\input\\steam_input_manifest.vdf";
	if (!SteamInput()->SetInputActionManifestFilePath(input_action_manifest_path.c_str())) {
		NANI_LOG_ERROR(m_logger, "Could not set Action Manifest file path!");
		NANI_LOG_ERROR(m_logger, "Path: {}", input_action_manifest_path);
	}
	SteamInput()->EnableDeviceCallbacks();
	init_steam_action_sets();
	setup_action_handles();

	// load keyboard bindings
	auto controls = dj::Json::from_file((finder.resource_path() + "/data/config/control_map.json").c_str());
	if (!controls) {
		NANI_LOG_ERROR(m_logger, "Failed to load input data for InputSystem.");
		return;
	}
	auto controls_result = std::move(*controls);
	assert(controls_result["controls"] && controls_result["controls"].is_object());
	for (auto const& [key, item] : controls_result["controls"].as_object()) {
		assert(item.is_object());
		if (item.as_object().contains("primary_key")) { set_primary_keyboard_binding(action_from_string(key), scancode_from_string(item["primary_key"].as_string())); }
	}
}

void InputSystem::init_steam_action_sets() {
	m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)] = SteamInput()->GetActionSetHandle("Platformer");
	m_steam_action_sets[static_cast<size_t>(ActionSet::Inventory)] = SteamInput()->GetActionSetHandle("Inventory");
	m_steam_action_sets[static_cast<size_t>(ActionSet::Map)] = SteamInput()->GetActionSetHandle("Map");
	m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)] = SteamInput()->GetActionSetHandle("Menu");
}

void InputSystem::setup_action_handles() {
#define XSTR(a) STR(a)
#define STR(a) #a
#define DEFINE_ACTION(action_name) m_digital_actions.insert({DigitalAction::action_name, {SteamInput()->GetDigitalActionHandle(XSTR(action_name)), sf::Keyboard::Scancode::Unknown, sf::Keyboard::Scancode::Unknown}});

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
	m_analog_actions.insert({AnalogAction::map_movement, {SteamInput()->GetAnalogActionHandle("map_movement")}});
	m_analog_actions.insert({AnalogAction::platformer_movement, {SteamInput()->GetAnalogActionHandle("platformer_movement")}});

#undef DEFINE_ACTION
#undef STR
#undef XSTR
}

void InputSystem::handle_event(std::optional<sf::Event> const event) {
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		m_last_device_used = InputDevice::keyboard;
		if (key_pressed->scancode != sf::Keyboard::Scancode::Unknown) { keys_pressed.insert(key_pressed->scancode); }
	} else if (auto const* key_released = event->getIf<sf::Event::KeyReleased>()) {
		keys_pressed.erase(key_released->scancode);
	}
}

void InputSystem::update() {
	if (!SteamInput()) { NANI_LOG_ERROR(m_logger, "SteamInput not initialized!"); }
	update_steam_controllers();
	SteamInput()->RunFrame();
	gather_raw_input();
	resolve_input();
}

void InputSystem::gather_raw_input() {
	// -----------------------------
	// 1) Clear per-frame scratch
	// -----------------------------
	for (auto& state : m_raw_digital) {
		state.held = false;
		state.active = false;
	}
	for (auto& state : m_raw_analog) {
		state.x = 0.f;
		state.y = 0.f;
		state.active = false;
	}

	// -----------------------------
	// 2) Digital actions (keyboard + gamepad)
	// -----------------------------
	for (int i = 0; i < static_cast<int>(DigitalAction::END); ++i) {
		auto action = static_cast<DigitalAction>(i);
		auto& raw = m_raw_digital[i];

		// --- Keyboard input ---
		auto primary = get_primary_keyboard_binding(action);
		auto secondary = get_secondary_keyboard_binding(action);

		raw.held = keys_pressed.contains(primary) || keys_pressed.contains(secondary);
		if (raw.held && m_controller_handle == 0) {
			raw.active = true; // keyboard is always valid
		}

		// --- Gamepad input ---
		if (m_controller_handle != 0) {
			auto data = SteamInput()->GetDigitalActionData(m_controller_handle, steam_handle_for(action));

			raw.held |= data.bState;
			raw.active |= data.bActive; // only true if action in current action set
		}
	}

	// -----------------------------
	// 3) Analog actions (sticks / triggers)
	// -----------------------------
	for (int i = 0; i < static_cast<int>(AnalogAction::END); ++i) {
		auto action = static_cast<AnalogAction>(i);
		auto& raw = m_raw_analog[i];

		if (m_controller_handle != 0) {
			auto data = SteamInput()->GetAnalogActionData(m_controller_handle, steam_handle_for(action));

			raw.x = data.x;
			raw.y = data.y;
			raw.active = data.bActive;
		}
		// optional: add keyboard analog support if desired
	}
}

void InputSystem::resolve_input() {
	// -----------------------------
	// 1) Digital actions
	// -----------------------------
	for (int i = 0; i < static_cast<int>(DigitalAction::END); ++i) {
		auto action = static_cast<DigitalAction>(i);
		auto& raw = m_raw_digital[i];
		auto& state = m_resolved_digital[i];

		// Only consider actions that are active in the current action set
		if (!raw.active) {
			state.triggered = false;
			state.released = state.held; // if it was held, now it's released
			state.held = false;
			continue;
		}

		bool pressed = raw.held;

		// Edge detection
		state.triggered = pressed && !state.held;
		state.released = !pressed && state.held;
		state.held = pressed;

		// Track last device used (for optional device switching logic)
		if (pressed) {
			if (m_controller_handle != 0 && raw.active) {
				m_last_device_used = InputDevice::gamepad;
			} else {
				m_last_device_used = InputDevice::keyboard;
			}
		}
	}

	// -----------------------------
	// 2) Analog actions
	// -----------------------------
	for (int i = 0; i < static_cast<int>(AnalogAction::END); ++i) {
		auto action = static_cast<AnalogAction>(i);
		auto& raw = m_raw_analog[i];
		auto& state = m_resolved_analog[i];

		if (raw.active) {
			state.x = raw.x;
			state.y = raw.y;
		} else {
			state.x = 0.f;
			state.y = 0.f;
		}
	}
}

void InputSystem::update_steam_controllers() {
	// No SteamInput → no controller
	if (!SteamInput()) {
		m_controller_handle = 0;
		return;
	}

	// Query connected controllers
	InputHandle_t controllers[STEAM_INPUT_MAX_COUNT]{};
	int const count = SteamInput()->GetConnectedControllers(controllers);

	// No controllers connected
	if (count == 0) {
		m_controller_handle = 0;
		return;
	}

	// Prefer existing controller if still connected
	if (m_controller_handle != 0) {
		for (int i = 0; i < count; ++i) {
			if (controllers[i] == m_controller_handle) {
				// Still valid — nothing to do
				return;
			}
		}
	}

	// Otherwise, pick the first connected controller
	m_controller_handle = controllers[0];

	// Activate current action set on the new controller
	activate_action_set_if_needed();
}

void InputSystem::set_action_set(ActionSet new_set) {
	if (m_active_action_set == new_set) { return; }
	m_active_action_set = new_set;

	// If a controller is connected, activate the corresponding SteamInput action set
	activate_action_set_if_needed();
}

void InputSystem::activate_action_set_if_needed() {
	if (!m_controller_handle) { return; }
	auto const steam_set = m_steam_action_sets[static_cast<size_t>(m_active_action_set)];
	if (!steam_set) { return; }
	SteamInput()->ActivateActionSet(m_controller_handle, steam_set);
}

void InputSystem::set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (m_digital_actions.contains(action)) { m_digital_actions.at(action).primary_binding = key; }
}
void InputSystem::set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (m_digital_actions.contains(action)) { m_digital_actions.at(action).secondary_binding = key; }
}

auto InputSystem::get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return m_digital_actions.at(action).primary_binding; }

auto InputSystem::get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return m_digital_actions.at(action).secondary_binding; }

auto InputSystem::get_joystick_throttle() const -> sf::Vector2f {
	if (last_device_used() != InputDevice::gamepad) { return {}; }
	return m_joystick_throttle;
}

auto InputSystem::get_i_joystick_throttle(bool exclusive) const -> sf::Vector2i {
	if (last_device_used() != InputDevice::gamepad) { return {}; }
	auto ret = sf::Vector2i{static_cast<int>(std::ceil(m_joystick_throttle.x)), static_cast<int>(std::ceil(m_joystick_throttle.y))};
	if (exclusive) {
		if (abs(m_joystick_throttle.x) > abs(m_joystick_throttle.y)) { ret.y = 0.f; }
		if (abs(m_joystick_throttle.x) <= abs(m_joystick_throttle.y)) { ret.x = 0.f; }
	}
	return ret;
}

void InputSystem::set_joystick_throttle(sf::Vector2f throttle) {
	// constrict throttle based on stick sensitivity
	m_joystick_throttle.x = std::clamp(throttle.x / 100.f, -1.f, 1.f);
	m_joystick_throttle.y = std::clamp(throttle.y / 100.f, -1.f, 1.f);
	if (std::abs(m_joystick_throttle.x) < m_stick_sensitivity) { m_joystick_throttle.x = 0.f; }
	if (std::abs(m_joystick_throttle.y) < m_stick_sensitivity) { m_joystick_throttle.y = 0.f; }
}

void InputSystem::handle_gamepad_connection(SteamInputDeviceConnected_t* data) {
	NANI_LOG_INFO(m_logger, "Connected controller with handle [{}]", data->m_ulConnectedDeviceHandle);
	controller_handle = data->m_ulConnectedDeviceHandle;
	last_controller_ty_used = InputDevice::gamepad; // Quickly switch to gamepad input
	setup_action_handles();
	set_action_set(active_action_set);
}

void InputSystem::handle_gamepad_disconnection(SteamInputDeviceDisconnected_t* data) {
	NANI_LOG_INFO(m_logger, "Disconnected controller with handle [{}] ", data->m_ulDisconnectedDeviceHandle);
	if (is_gamepad_connected()) { set_flag(InputSystemFlags::gamepad_disconnected); }
	m_controller_handle = 0;
	m_last_device_used = InputDevice::keyboard; // Quickly switch to keyboard input
}

void InputSystem::open_bindings_overlay() const {
	if (!is_gamepad_connected()) { return; }
	SteamInput()->ShowBindingPanel(m_controller_handle);
}

bool InputSystem::process_gamepad_disconnection() {
	auto ret = has_flag_set(InputSystemFlags::gamepad_disconnected);
	set_flag(InputSystemFlags::gamepad_disconnected, false);
	return ret;
}

} // namespace fornani::input
