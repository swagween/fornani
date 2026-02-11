
#pragma once

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/systems/InputActionMap.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <unordered_set>

namespace fornani {
class Game;
}

namespace fornani::automa {
class ControlsMenu;
class SettingsMenu;
} // namespace fornani::automa

namespace fornani::data {
class DataManager;
}

namespace fornani::input {

enum class ActionSet { Platformer, Menu, END };
enum class DigitalActionQueryType { held, triggered, released };
enum class InputDevice { none, keyboard, gamepad };
enum class InputSystemSettings { gamepad_input_enabled, auto_sprint };
enum class InputSystemFlags { gamepad_disconnected, keyboard_input_detected, changed_action_sets, key_was_pressed };

// raw input
struct RawDigitalState {
	bool held = false;	 // button/key is down
	bool active = false; // valid in current action set
};

struct RawAnalogState {
	float x = 0.f;
	float y = 0.f;
	float prev_x = 0.f;
	float prev_y = 0.f;
	bool active = false;
};

// processed input
struct ResolvedDigitalState {
	bool triggered = false; // pressed this frame
	bool held = false;		// currently held
	bool released = false;	// released this frame
	bool locked = false;	// prevents retrigger until release
};

struct ResolvedAnalogState {
	float prev_x = 0.f;
	float prev_y = 0.f;
	bool active = false;
	float x = 0.f;
	float y = 0.f;
};

// input storage units
struct DigitalActionData {
	InputHandle_t steam_handle; // handle to SteamInput action
	sf::Keyboard::Scancode primary_binding;
	sf::Keyboard::Scancode secondary_binding;
};
struct AnalogActionData {
	InputHandle_t steam_handle; // handle to SteamInput analog action
	float x = 0.f;				// -1.0 … 1.0
	float y = 0.f;				// -1.0 … 1.0
	bool active = false;		// is stick being moved at all?
	float magnitude = 0.f;		// optional, sqrt(x^2 + y^2)
};

// for helptext
struct DigitalActionSource {
	EInputActionOrigin controller_origin{};
	sf::Keyboard::Scancode key{};
};

// --------------------------------------------------
// Input system
// --------------------------------------------------

class InputSystem final : public Flaggable<InputSystemFlags> {
  public:
	friend class Game;
	friend class automa::ControlsMenu;
	friend class automa::SettingsMenu;
	friend class data::DataManager;

	InputSystem(ResourceFinder& finder);
	void handle_event(std::optional<sf::Event> const event);
	void update(); // calls gather + resolve
	void flush_inputs();

	// --- Action set control ---
	void set_action_set(ActionSet set);
	[[nodiscard]] auto get_active_action_set() const -> ActionSet { return m_active_action_set; };

	// --- Gameplay queries ---
	[[nodiscard]] auto digital(DigitalAction action) const -> ResolvedDigitalState const& { return m_resolved_digital[static_cast<size_t>(action)]; }
	[[nodiscard]] auto analog(AnalogAction action) const -> ResolvedAnalogState const& { return m_resolved_analog[static_cast<size_t>(action)]; }
	[[nodiscard]] auto direction_triggered(AnalogAction action, MoveDirection dir) const -> bool;
	[[nodiscard]] auto direction_held(AnalogAction action, MoveDirection dir) const -> bool;
	[[nodiscard]] auto direction_released(AnalogAction action, MoveDirection dir) const -> bool;
	[[nodiscard]] auto query_direction(AnalogAction action, MoveDirection dir, DigitalActionQueryType type) const -> bool;
	[[nodiscard]] auto menu_move(MoveDirection dir, DigitalActionQueryType type = DigitalActionQueryType::triggered) const -> bool;
	[[nodiscard]] auto is_any_direction_held(AnalogAction action) const -> bool;

	// --- Settings ---
	void set_joystick_sensitivity(float to) { m_stick_sensitivity = std::clamp(to, 0.f, 1.f); }
	[[nodiscard]] auto last_device_used() const -> InputDevice { return m_last_device_used; }
	[[nodiscard]] auto is_gamepad_connected() const -> bool { return m_controller_handle != 0; }
	[[nodiscard]] auto is_gamepad_input_enabled() const -> bool { return m_settings.test(InputSystemSettings::gamepad_input_enabled); }
	[[nodiscard]] auto is_autosprint_enabled() const -> bool { return m_settings.test(InputSystemSettings::auto_sprint); }

	// --- Joystick queries ---
	[[nodiscard]] auto get_joystick_throttle(AnalogAction action = AnalogAction::move) const -> sf::Vector2f;
	[[nodiscard]] auto get_i_joystick_throttle(bool exclusive) const -> sf::Vector2i;
	void set_joystick_throttle(sf::Vector2f throttle);

	// --- Keyboard queries ---
	[[nodiscard]] auto was_keyboard_input_detected() const -> bool { return has_flag_set(InputSystemFlags::keyboard_input_detected); }
	[[nodiscard]] auto get_last_key_pressed() const -> sf::Keyboard::Scancode { return m_last_key_pressed; };

	// --- Binding queries ---
	[[nodiscard]] auto get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;
	[[nodiscard]] auto get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;
	[[nodiscard]] auto get_digital_action_source_name(DigitalAction action) const -> std::string_view;
	[[nodiscard]] auto get_digital_action_source(DigitalAction action) const -> DigitalActionSource;
	[[nodiscard]] auto get_icon_lookup_by_action(DigitalAction action) const -> sf::Vector2i;

	/// @return Returns true if keyboard control bindings have a duplicate.
	[[nodiscard]] auto has_forbidden_duplicate_binding() const -> bool;
	[[nodiscard]] auto is_bound_to_same_input(DigitalAction first, DigitalAction second) const -> bool;
	[[nodiscard]] auto is_gamepad() const -> bool { return m_last_device_used == InputDevice::gamepad; }
	[[nodiscard]] auto is_keyboard() const -> bool { return m_last_device_used == InputDevice::keyboard; }
	[[nodiscard]] auto get_controller_handle() const -> InputHandle_t { return m_controller_handle; }

	// --- Gamepad handling ---
	void open_bindings_overlay() const;

	/// @brief Processes gamepad disconnection to pause the game.
	/// @return Returns true if gamepad was just disconnected, otherwise returns false.
	bool process_gamepad_disconnection();

  private:
	// --- General ---
	void set_setting(InputSystemSettings setting, bool on = true) { on ? m_settings.set(setting) : m_settings.reset(setting); }

	// ---- Phase 1 ----
	void gather_raw_input();

	// ---- Phase 2 ----
	void resolve_input();

	// ---- SteamInput helpers ----
	void update_steam_controllers();
	void set_steam_action_set(InputActionSetHandle_t to_set);
	void init_steam_action_sets();
	void setup_action_handles();
	void set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);
	void set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);
	bool is_action_allowed(DigitalAction action) const;
	bool is_action_allowed(AnalogAction action) const;
	float analog_axis_value(ResolvedAnalogState const& a, MoveDirection dir, bool previous = false) const;
	bool query_digital_axis(MoveDirection dir, DigitalActionQueryType type, ActionSet set = ActionSet::Platformer) const;

	[[nodiscard]] auto steam_handle_for(DigitalAction action) const -> InputHandle_t { return m_digital_actions.at(action).steam_handle; }
	[[nodiscard]] auto steam_handle_for(AnalogAction action) const -> InputHandle_t { return m_analog_actions.at(action).steam_handle; }

	// --- Keyboard helpers ---
	void set_last_key_pressed(sf::Keyboard::Scancode to_key);
	void load_keyboard_controls(ResourceFinder& finder);

  private:
	// --- Settings ---
	util::BitFlags<InputSystemSettings> m_settings{};

	// --- State ---
	ActionSet m_active_action_set{ActionSet::Platformer};
	InputDevice m_last_device_used{InputDevice::none};
	std::unordered_map<DigitalAction, DigitalActionData> m_digital_actions{};
	std::unordered_map<AnalogAction, AnalogActionData> m_analog_actions{};
	sf::Keyboard::Scancode m_last_key_pressed{};

	// --- SteamInput ---
	InputHandle_t m_controller_handle{0};
	std::array<InputActionSetHandle_t, static_cast<size_t>(ActionSet::END)> m_steam_action_sets{};

	// --- Joystick Input ---
	sf::Vector2f m_joystick_throttle{};
	float m_stick_sensitivity;

	// --- Raw (per-frame scratch) ---
	std::array<RawDigitalState, static_cast<size_t>(DigitalAction::END)> m_raw_digital;
	std::array<RawAnalogState, static_cast<size_t>(AnalogAction::END)> m_raw_analog;
	std::unordered_set<sf::Keyboard::Scancode> keys_pressed;

	// --- Resolved (persistent) ---
	std::array<ResolvedDigitalState, static_cast<size_t>(DigitalAction::END)> m_resolved_digital;
	std::array<ResolvedAnalogState, static_cast<size_t>(AnalogAction::END)> m_resolved_analog;

	io::Logger m_logger{"Input"};

	STEAM_CALLBACK(InputSystem, handle_gamepad_connection, SteamInputDeviceConnected_t);
	STEAM_CALLBACK(InputSystem, handle_gamepad_disconnection, SteamInputDeviceDisconnected_t);
};

} // namespace fornani::input
