
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>

#include <cmath>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {
class Game;
}

namespace fornani::config {

enum class DigitalAction : int {
	// Platformer controls
	platformer_left,
	platformer_right,
	platformer_up,
	platformer_down,
	platformer_jump,
	platformer_shoot,
	platformer_sprint,
	platformer_slide,
	platformer_dash,
	platformer_inspect,
	platformer_arms_switch_left,
	platformer_arms_switch_right,
	platformer_open_inventory,
	platformer_toggle_pause,

	// Inventory controls
	inventory_close,

	// Menu controls
	menu_left,
	menu_right,
	menu_up,
	menu_down,
	menu_select,
	menu_cancel,
	menu_tab_left,
	menu_tab_right,
	menu_confirm,

	COUNT
};

enum class AnalogAction { platformer_movement, map_movement };

enum class ActionSet {
	Platformer,
	Inventory,
	Map,
	Menu,
};

enum class ControllerStatus { gamepad_connected };

struct DigitalActionSource {
	EInputActionOrigin controller_origin{};
	sf::Keyboard::Scancode key{};
};

struct DigitalActionStatus {
	explicit DigitalActionStatus(DigitalAction const action) : action(action) {}
	DigitalAction action{};
	bool held{};
	bool triggered{};
	bool released{};
};

struct AnalogActionStatus {
	explicit AnalogActionStatus(AnalogAction const action) : action(action) {}
	AnalogAction action{};

	float x{};
	float y{};
};

enum class InputDevice { none, keyboard, gamepad };

struct RawDigitalInput {
	bool pressed = false; // this frame
	bool held = false;	  // down
	bool active = false;  // bound / valid
};

struct ResolvedActionState {
	bool triggered = false;
	bool held = false;
	bool released = false;
};

class ControllerMap {
  public:
	friend class fornani::Game;
	explicit ControllerMap(automa::ServiceProvider& svc);

	ControllerMap(ControllerMap const&) = delete;
	ControllerMap operator=(ControllerMap const&) = delete;

	/// @brief Update internal action states. Call once on tick update, and nowhere else.
	void update();

	/// @brief Process the SFML event given.
	/// @details Used for keyboard controls.
	void handle_event(std::optional<sf::Event> const event);

	void flush_inputs();

	sf::Vector2i get_icon_lookup_by_action(DigitalAction action) const;

	/// @brief Returns whether there is a gamepad connected or not.
	[[nodiscard]] auto gamepad_connected() const -> bool { return controller_handle != 0; }
	[[nodiscard]] auto gamepad_disconnected() const -> bool { return controller_handle == 0; }
	[[nodiscard]] auto digital_action_status(DigitalAction action) -> DigitalActionStatus;
	[[nodiscard]] auto analog_action_status(AnalogAction action) const -> AnalogActionStatus { return analog_actions.at(action).second; }
	[[nodiscard]] auto get_action_set() const -> ActionSet { return active_action_set; }
	[[nodiscard]] auto digital_action_name(DigitalAction action) const -> std::string_view;
	[[nodiscard]] auto digital_action_source(DigitalAction action) const -> DigitalActionSource;
	[[nodiscard]] auto digital_action_source_name(DigitalAction action) const -> std::string_view;
	/// @brief Set the current action set.
	/// @warning This determines the actions capable to be received by the connected gamepads, so remember to set it correctly!
	void set_action_set(ActionSet set);
	/// @brief Open the Steam controller configuration overlay.
	void open_bindings_overlay() const;

	auto actions_queried_this_update() const -> std::unordered_set<DigitalAction> const& { return m_actions_queried_this_update; }

	void set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);
	void set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);
	/// @brief Returns the primary keyboard key associated with a particular action.
	/// @param action
	/// @return The key bound, or sf::Keyboard::Scancode::Unknown if no key was bound.
	[[nodiscard]] auto get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;
	/// @brief Returns the secondary keyboard key associated with a particular action.
	/// @param action
	/// @return The key bound, or sf::Keyboard::Scancode::Unknown if no key was bound.
	[[nodiscard]] auto get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;

	[[nodiscard]] auto key_to_string(sf::Keyboard::Scancode) const -> std::string_view;
	[[nodiscard]] auto string_to_key(std::string_view) const -> sf::Keyboard::Scancode;

	[[nodiscard]] auto last_controller_type_used() const -> InputDevice { return last_controller_ty_used; }
	[[nodiscard]] auto is_gamepad() const -> bool { return last_controller_ty_used == InputDevice::gamepad; }
	[[nodiscard]] auto is_keyboard() const -> bool { return last_controller_ty_used == InputDevice::keyboard; }

	/// @return Returns true if keyboard control bindings have a duplicate.
	[[nodiscard]] auto has_forbidden_duplicate_binding() const -> bool;
	[[nodiscard]] auto was_keyboard_input_detected() const -> bool { return m_keyboard_input_detected; }
	[[nodiscard]] auto is_bound_to_same_input(DigitalAction first, DigitalAction second) const -> bool;

	/// @brief Processes gamepad disconnection to pause the game.
	/// @return Returns true if gamepad was just disconnected, otherwise returns false.
	bool process_gamepad_disconnection();

	// TODO Move autosprint and tutorial toggles away from ControllerMap
	void enable_autosprint(bool enable) { autosprint_enabled = enable; }
	void enable_gamepad_input(bool enable) { gamepad_input_enabled = enable; }
	[[nodiscard]] auto is_gamepad_input_enabled() const -> bool { return gamepad_input_enabled; }
	[[nodiscard]] auto is_autosprint_enabled() const -> bool { return autosprint_enabled; }

	/// @brief Obtains a `DigitalAction` variant by its name in the enum.
	auto get_action_by_identifier(std::string_view id) -> DigitalAction;
	[[nodiscard]] auto get_joystick_throttle() const -> sf::Vector2f;
	[[nodiscard]] auto get_i_joystick_throttle(bool exclusive) const -> sf::Vector2i;

	[[nodiscard]] auto get_last_key_pressed() const -> sf::Keyboard::Scancode { return m_last_key_pressed; };

	void set_joystick_throttle(sf::Vector2f throttle);

  private:
	void set_last_key_pressed(sf::Keyboard::Scancode to_key);
	void set_keyboard_input_detected(bool flag);
	struct DigitalActionData {
		InputDigitalActionHandle_t steam_handle;
		DigitalActionStatus status;
		sf::Keyboard::Scancode primary_binding;
		sf::Keyboard::Scancode secondary_binding;

		bool was_active_last_tick{};
	};
	struct {
		bool gamepad_disconnected{};
	} out{};
	std::unordered_map<DigitalAction, DigitalActionData> digital_actions{};
	std::unordered_map<DigitalAction, std::string> digital_action_names{};
	std::unordered_map<AnalogAction, std::pair<InputAnalogActionHandle_t, AnalogActionStatus>> analog_actions{};
	std::unordered_set<sf::Keyboard::Scancode> keys_pressed;
	InputActionSetHandle_t platformer_action_set{};
	InputActionSetHandle_t menu_action_set{};
	InputActionSetHandle_t inventory_action_layer{};
	InputActionSetHandle_t map_action_layer{};

	std::unordered_set<DigitalAction> m_actions_queried_this_update;

	InputDevice last_controller_ty_used{InputDevice::keyboard};

	ActionSet active_action_set{};

	InputHandle_t controller_handle{};

	sf::Keyboard::Scancode m_last_key_pressed{};

	bool gamepad_input_enabled{true};
	bool autosprint_enabled{true};
	bool m_keyboard_input_detected{};

	// joystick members
	sf::Vector2f m_joystick_throttle{};
	float m_stick_sensitivity;

	void reset_digital_action_states();

	/// @brief Sets up the values of `digital_actions` and `analog_actions` via the Steam Input API.
	/// @details Since the Steam Input API returns invalid handles if no gamepad is connected (bug?), this needs to be recalled every time a controller is connected.
	void setup_action_handles();

	using RawInputMap = std::unordered_map<DigitalAction, RawDigitalInput>;
	using ResolvedActionMap = std::unordered_map<DigitalAction, ResolvedActionState>;

	RawInputMap raw_inputs;

	io::Logger m_logger{"config"};

	STEAM_CALLBACK(ControllerMap, handle_gamepad_connection, SteamInputDeviceConnected_t);

	STEAM_CALLBACK(ControllerMap, handle_gamepad_disconnection, SteamInputDeviceDisconnected_t);
};

} // namespace fornani::config
