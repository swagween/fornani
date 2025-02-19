
#pragma once

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
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
	// XXX platformer_dash
	platformer_shield,
	platformer_inspect,
	platformer_arms_switch_left,
	platformer_arms_switch_right,
	platformer_open_inventory,
	platformer_open_map,
	platformer_toggle_pause,

	// Inventory controls
	inventory_open_map,
	inventory_close,

	// Map controls
	map_open_inventory,
	map_close,

	// Menu controls
	// XXX rename to UI
	menu_left,
	menu_right,
	menu_up,
	menu_down,
	menu_select,
	menu_cancel,
	menu_switch_left,
	menu_switch_right,

	COUNT
};

enum class AnalogAction : uint8_t {
	// Map controls
	map_movement,
};

enum class ActionSet : uint8_t {
	Platformer,
	Inventory,
	Map,
	Menu,
};

enum class ControllerType : uint8_t { keyboard, gamepad };
enum class ControllerStatus : uint8_t { gamepad_connected };

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

class ControllerMap {
  public:
	explicit ControllerMap(automa::ServiceProvider& svc);

	ControllerMap(ControllerMap const&) = delete;
	ControllerMap operator=(ControllerMap const&) = delete;

	/// @brief Update internal action states. Call once on tick update, and nowhere else.
	void update();

	/// @brief Process the SFML event given.
	/// @details Used for keyboard controls.
	void handle_event(std::optional<sf::Event> const event);

	/// @brief Returns whether there is a gamepad connected or not.
	[[nodiscard]] auto gamepad_connected() const -> bool { return controller_handle != 0; }
	[[nodiscard]] auto gamepad_disconnected() const -> bool { return controller_handle == 0; }
	[[nodiscard]] auto digital_action_status(DigitalAction action) -> DigitalActionStatus;
	[[nodiscard]] auto analog_action_status(AnalogAction action) const -> AnalogActionStatus { return analog_actions.at(action).second; }
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

	[[nodiscard]] auto last_controller_type_used() const -> ControllerType { return last_controller_ty_used; }

	/// @brief Processes gamepad disconnection to pause the game.
	/// @return Returns true if gamepad was just disconnected, otherwise returns false.
	bool process_gamepad_disconnection();

	// TODO Move autosprint and tutorial toggles away from ControllerMap
	void enable_autosprint(bool enable) { autosprint_enabled = enable; }
	void enable_gamepad_input(bool enable) { gamepad_input_enabled = enable; }
	[[nodiscard]] auto is_gamepad_input_enabled() const -> bool { return gamepad_input_enabled; }
	[[nodiscard]] auto is_autosprint_enabled() const -> bool { return autosprint_enabled; }

	/// @brief Obtains a `DigitalAction` variant by its name in the enum.
	auto get_action_by_identifier(std::string_view id) -> config::DigitalAction;

  private:
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

	ControllerType last_controller_ty_used{ControllerType::keyboard};

	ActionSet active_action_set{};

	InputHandle_t controller_handle{};

	bool gamepad_input_enabled{true};
	bool autosprint_enabled{true};

	void reset_digital_action_states();

	/// @brief Sets up the values of `digital_actions` and `analog_actions` via the Steam Input API.
	/// @details Since the Steam Input API returns invalid handles if no gamepad is connected (bug?), this needs to be recalled every time a controller is connected.
	void setup_action_handles();

	STEAM_CALLBACK(ControllerMap, handle_gamepad_connection, SteamInputDeviceConnected_t);

	STEAM_CALLBACK(ControllerMap, handle_gamepad_disconnection, SteamInputDeviceDisconnected_t);
};

} // namespace fornani::config
