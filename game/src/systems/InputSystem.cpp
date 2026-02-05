
#include <djson/json.hpp>
#include <fornani/gui/ActionControlIconQuery.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani::input {

constexpr auto default_joystick_sensitivity_v = 0.2f;
constexpr auto analog_press_threshold_v = 0.3f;
constexpr auto analog_release_threshold_v = 0.25f;

static auto get_action_set_from_action(AnalogAction action) -> ActionSet {
	switch (action) {
	case AnalogAction::move:
	case AnalogAction::pan: return ActionSet::Platformer;
	case AnalogAction::map_pan: return ActionSet::Menu;
	}
	return ActionSet::Platformer;
}

static auto get_action_set_from_action(DigitalAction action) -> ActionSet {
	switch (action) {
	case DigitalAction::jump:
	case DigitalAction::shoot:
	case DigitalAction::sprint:
	case DigitalAction::slide:
	case DigitalAction::dash:
	case DigitalAction::inspect:
	case DigitalAction::tab_left:
	case DigitalAction::tab_right:
	case DigitalAction::inventory:
	case DigitalAction::pause: return ActionSet::Platformer;

	case DigitalAction::menu_close:
	case DigitalAction::menu_left:
	case DigitalAction::menu_right:
	case DigitalAction::menu_up:
	case DigitalAction::menu_down:
	case DigitalAction::menu_select:
	case DigitalAction::menu_tab_right:
	case DigitalAction::menu_tab_left:
	case DigitalAction::menu_back:
	case DigitalAction::menu_confirm: return ActionSet::Menu;

	case DigitalAction::END:
	default: assert(false && "Invalid action set in get_action_set_from_action");
	}
	assert(false && "Invalid action set in get_action_set_from_action");
	return ActionSet::Platformer; // This will never be hit
}

InputSystem::InputSystem(ResourceFinder& finder) : m_stick_sensitivity{default_joystick_sensitivity_v} {
	NANI_LOG_INFO(m_logger, "Initializing Steam Input");
	if (!SteamInput()->Init(true)) {
		NANI_LOG_WARN(m_logger, "Could not initialize Steam Input!");
	} else {
		NANI_LOG_INFO(m_logger, "Steam Input initialized");
	}
	std::string input_action_manifest_path = finder.resource_path() + "/text/input/steam_input_manifest.vdf";
	if (!std::filesystem::exists(input_action_manifest_path)) { NANI_LOG_ERROR(m_logger, "Manifest file does not exist on disk!"); }
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
	InputHandle_t handles[STEAM_INPUT_MAX_COUNT];
	int count = SteamInput()->GetConnectedControllers(handles);
	NANI_LOG_INFO(m_logger, "Available Steam Input Controllers: {}", count);
}

void InputSystem::init_steam_action_sets() {
	m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)] = SteamInput()->GetActionSetHandle("Platformer");
	m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)] = SteamInput()->GetActionSetHandle("Menu");
}

void InputSystem::setup_action_handles() {
#define XSTR(a) STR(a)
#define STR(a) #a
#define DEFINE_ACTION(action_name) m_digital_actions.insert({DigitalAction::action_name, {SteamInput()->GetDigitalActionHandle(XSTR(action_name)), sf::Keyboard::Scancode::Unknown, sf::Keyboard::Scancode::Unknown}});

	// Platformer controls
	DEFINE_ACTION(up);
	DEFINE_ACTION(down);
	DEFINE_ACTION(left);
	DEFINE_ACTION(right);
	DEFINE_ACTION(jump);
	DEFINE_ACTION(shoot);
	DEFINE_ACTION(sprint);
	DEFINE_ACTION(slide);
	DEFINE_ACTION(dash);
	DEFINE_ACTION(inspect);
	DEFINE_ACTION(tab_left);
	DEFINE_ACTION(tab_right);
	DEFINE_ACTION(inventory);
	DEFINE_ACTION(pause);

	// Inventory controls
	DEFINE_ACTION(inventory);

	// Menu controls
	DEFINE_ACTION(menu_left);
	DEFINE_ACTION(menu_right);
	DEFINE_ACTION(menu_up);
	DEFINE_ACTION(menu_down);
	DEFINE_ACTION(menu_select);
	DEFINE_ACTION(menu_tab_left);
	DEFINE_ACTION(menu_tab_right);
	DEFINE_ACTION(menu_back);
	DEFINE_ACTION(menu_confirm);
	DEFINE_ACTION(menu_close);

	// Analog actions
	m_analog_actions.insert({AnalogAction::move, {SteamInput()->GetAnalogActionHandle("move")}});
	m_analog_actions.insert({AnalogAction::pan, {SteamInput()->GetAnalogActionHandle("pan")}});
	m_analog_actions.insert({AnalogAction::map_pan, {SteamInput()->GetAnalogActionHandle("map_pan")}});

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
	SteamInput()->RunFrame();
	gather_raw_input();
	resolve_input();
	update_steam_controllers();
}

void InputSystem::flush_inputs() {
	for (auto& state : m_resolved_digital) {
		state.held = false;
		state.triggered = false;
		state.released = false;
	}

	// Analog actions
	for (auto& state : m_resolved_analog) {
		state.x = 0.f;
		state.y = 0.f;
	}
	for (auto& raw : m_raw_digital) {
		raw.held = false;
		raw.active = false;
	}
	for (auto& raw : m_raw_analog) {
		raw.x = 0.f;
		raw.y = 0.f;
		raw.active = false;
	}
}

bool InputSystem::is_action_allowed(DigitalAction action) const { return get_action_set_from_action(action) == m_active_action_set; }

bool InputSystem::is_action_allowed(AnalogAction action) const { return get_action_set_from_action(action) == m_active_action_set; }

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

		bool keyboard_pressed = keys_pressed.contains(primary) || keys_pressed.contains(secondary);

		// --- Gamepad input ---
		bool gamepad_pressed = false;
		if (m_controller_handle != 0) {
			auto data = SteamInput()->GetDigitalActionData(m_controller_handle, steam_handle_for(action));
			gamepad_pressed = data.bState;
		}
		raw.held = keyboard_pressed || gamepad_pressed;
		raw.active = is_action_allowed(action);
		if (keyboard_pressed) { m_last_device_used = InputDevice::keyboard; }
		if (gamepad_pressed) { m_last_device_used = InputDevice::gamepad; }
	}

	// -----------------------------
	// 3) Analog actions (sticks / triggers)
	// -----------------------------
	for (int i = 0; i < static_cast<int>(AnalogAction::END); ++i) {
		auto action = static_cast<AnalogAction>(i);
		auto& raw = m_raw_analog[i];

		raw.prev_x = raw.x;
		raw.prev_y = raw.y;

		if (m_controller_handle != 0) {
			auto data = SteamInput()->GetAnalogActionData(m_controller_handle, steam_handle_for(action));

			raw.x = data.x;
			raw.y = -data.y; // negative because steam's vertical axis is inverted (???)
		}

		raw.active = is_action_allowed(action);
	}
}

void InputSystem::resolve_input() {
	for (int i = 0; i < static_cast<int>(DigitalAction::END); ++i) {
		auto action = static_cast<DigitalAction>(i);
		auto& raw = m_raw_digital[i];
		auto& state = m_resolved_digital[i];

		bool pressed = raw.held;

		// Update release first
		state.released = !pressed && state.held;
		if (state.released) {
			state.locked = false; // unlock when the button is released
		}

		// Triggered only if not locked
		if (pressed && !state.held && !state.locked) {
			state.triggered = true;
			state.locked = true; // lock until release
		} else {
			state.triggered = false;
		}

		// Update held
		state.held = pressed;
		if (!is_action_allowed(action)) { state = {}; }
		if (has_flag_set(InputSystemFlags::changed_action_sets)) {
			state.triggered = false;
			state.held = false;
		}
	}
	set_flag(InputSystemFlags::changed_action_sets, false);

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

	static InputHandle_t active_controller = 0;

	InputHandle_t controllers[STEAM_INPUT_MAX_COUNT];
	int count = SteamInput()->GetConnectedControllers(controllers);

	if (count > 0) {
		if (active_controller != controllers[0]) {
			active_controller = controllers[0];
			NANI_LOG_INFO(m_logger, "Activated Controller {}.", active_controller);
			// IMPORTANT: re-activate action set
			SteamInput()->ActivateActionSet(active_controller, m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)]);
		}
	}

	// No controllers connected
	if (count == 0) {
		m_controller_handle = 0;
		return;
	}

	// Otherwise, pick the first connected controller
	m_controller_handle = controllers[0];
	auto set = SteamInput()->GetCurrentActionSet(m_controller_handle);
	if (set == m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)] && m_active_action_set == ActionSet::Menu) { set_steam_action_set(m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)]); }
	if (set == m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)] && m_active_action_set == ActionSet::Platformer) { set_steam_action_set(m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)]); }
}

void InputSystem::set_action_set(ActionSet new_set) {
	if (m_active_action_set == new_set) { return; }
	m_active_action_set = new_set;
	set_flag(InputSystemFlags::changed_action_sets);
}

float InputSystem::analog_axis_value(ResolvedAnalogState const& a, MoveDirection dir, bool previous) const {
	auto query_x = previous ? a.prev_x : a.x;
	auto query_y = previous ? a.prev_y : a.y;
	switch (dir) {
	case MoveDirection::left: return -query_x;
	case MoveDirection::right: return query_x;
	case MoveDirection::up: return -query_y;
	case MoveDirection::down: return query_y;
	}
	return 0.f;
}

bool InputSystem::query_digital_axis(MoveDirection dir, DigitalActionQueryType type) const {
	auto iaction = static_cast<int>(DigitalAction::up) + static_cast<int>(dir);
	auto action = digital(static_cast<DigitalAction>(iaction));
	switch (type) {
	case DigitalActionQueryType::held: return action.held;
	case DigitalActionQueryType::triggered: return action.triggered;
	case DigitalActionQueryType::released: return action.triggered;
	}
	return false;
}

auto InputSystem::direction_triggered(AnalogAction action, MoveDirection dir) const -> bool {
	if (is_gamepad()) {
		auto const& a = analog(action);
		return a.active && analog_axis_value(a, dir, true) <= analog_press_threshold_v && analog_axis_value(a, dir) > analog_press_threshold_v;
	}
	return query_digital_axis(dir, DigitalActionQueryType::triggered);
}

auto InputSystem::direction_held(AnalogAction action, MoveDirection dir) const -> bool {
	if (is_gamepad()) { return analog_axis_value(analog(action), dir) > analog_press_threshold_v; }
	return query_digital_axis(dir, DigitalActionQueryType::held);
}

auto InputSystem::direction_released(AnalogAction action, MoveDirection dir) const -> bool {
	if (is_gamepad()) {
		auto const& a = analog(action);
		return a.active && analog_axis_value(a, dir, true) >= analog_release_threshold_v && analog_axis_value(a, dir) < analog_release_threshold_v;
	}
	return query_digital_axis(dir, DigitalActionQueryType::released);
}

void InputSystem::set_steam_action_set(InputActionSetHandle_t to_set) {
	if (!m_controller_handle) { return; }
	NANI_LOG_INFO(m_logger, "Setting controller action set...");
	set_flag(InputSystemFlags::changed_action_sets);
	SteamInput()->ActivateActionSet(m_controller_handle, to_set);
}

void InputSystem::set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (m_digital_actions.contains(action)) { m_digital_actions.at(action).primary_binding = key; }
}
void InputSystem::set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key) {
	if (m_digital_actions.contains(action)) { m_digital_actions.at(action).secondary_binding = key; }
}

auto InputSystem::get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return m_digital_actions.at(action).primary_binding; }

auto InputSystem::get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode { return m_digital_actions.at(action).secondary_binding; }

auto InputSystem::get_joystick_throttle() const -> sf::Vector2f { return {analog(AnalogAction::move).x, analog(AnalogAction::move).y}; }

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
	m_controller_handle = data->m_ulConnectedDeviceHandle;
	m_last_device_used = InputDevice::gamepad; // Quickly switch to gamepad input
	setup_action_handles();
	set_action_set(m_active_action_set);
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

auto InputSystem::has_forbidden_duplicate_binding() const -> bool {
	for (auto& binding : m_digital_actions) {
		for (auto& other : m_digital_actions) {
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

auto InputSystem::is_bound_to_same_input(DigitalAction first, DigitalAction second) const -> bool { return m_digital_actions.at(first).primary_binding == m_digital_actions.at(second).primary_binding; }

void InputSystem::set_last_key_pressed(sf::Keyboard::Scancode to_key) { m_last_key_pressed = to_key; }

[[nodiscard]] auto InputSystem::get_digital_action_source_name(DigitalAction action) const -> std::string_view {
	if (m_controller_handle) {
		auto action_set = get_action_set_from_action(action);
		InputActionSetHandle_t handle{};
		switch (action_set) {
		case ActionSet::Menu: handle = m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)]; break;
		case ActionSet::Platformer: handle = m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)]; break;
		}

		EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
		if (SteamInput()->GetDigitalActionOrigins(m_controller_handle, handle, m_digital_actions.at(action).steam_handle, origins) > 0) {
			return SteamInput()->GetStringForActionOrigin(origins[0]);
		} else {
			return "Unassigned";
		}
	} else {
		return string_from_scancode(m_digital_actions.at(action).primary_binding);
	}
}

[[nodiscard]] auto InputSystem::get_digital_action_source(DigitalAction action) const -> DigitalActionSource {
	auto controller_origin = k_EInputActionOrigin_None;
	if (m_controller_handle) {
		auto action_set = get_action_set_from_action(action);
		InputActionSetHandle_t handle{};
		switch (action_set) {
		case ActionSet::Menu: handle = m_steam_action_sets[static_cast<size_t>(ActionSet::Menu)]; break;
		case ActionSet::Platformer: handle = m_steam_action_sets[static_cast<size_t>(ActionSet::Platformer)]; break;
		}

		EInputActionOrigin origins[STEAM_INPUT_MAX_ORIGINS];
		if (SteamInput()->GetDigitalActionOrigins(m_controller_handle, handle, m_digital_actions.at(action).steam_handle, origins) > 0) {
			if (origins[0] < k_EInputActionOrigin_Count) { controller_origin = origins[0]; }
		}
	}
	return DigitalActionSource{.controller_origin = controller_origin, .key = get_primary_keyboard_binding(action)};
}

auto InputSystem::get_icon_lookup_by_action(DigitalAction action) const -> sf::Vector2i {
	auto source = get_digital_action_source(action);
	return (source.controller_origin == k_EInputActionOrigin_None) ? gui::get_key_coordinates(source.key) : gui::get_controller_button_coordinates(source.controller_origin);
}

} // namespace fornani::input
