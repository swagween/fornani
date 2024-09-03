
#pragma once

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {

enum class DigitalAction {
	// Platformer controls
	platformer_left,
	platformer_right,
	platformer_up,
	platformer_down,
	platformer_jump,
	platformer_shoot,
	platformer_sprint,
	platformer_shield,
	platformer_inspect,
	platformer_arms_switch_left,
	platformer_arms_switch_right,
	platformer_open_inventory,
	platformer_open_map,
	platformer_toggle_pause,

	// Inventory controls
	// XXX convert this into an UI action set layer
	inventory_left,
	inventory_right,
	inventory_up,
	inventory_down,
	inventory_close,

	// Map controls
	// XXX convert this into an UI action set layer
	map_close,

	// Menu controls
	// XXX rename to UI
	menu_up,
	menu_down,
	// XXX add left and right actions
	menu_select,
	menu_cancel
};

enum class AnalogAction {
	// Map controls
	map_movement,
};

enum class ActionSet {
	Platformer,
	Inventory,
	Map,
	Menu,
};

enum class ControllerType { keyboard, gamepad };
enum class ControllerStatus { gamepad_connected };
enum class Toggles { keyboard, gamepad, autosprint };

struct DigitalActionStatus {
	DigitalActionStatus(DigitalAction action) : action(action) {}
	DigitalAction action{};
	bool held{};
	bool triggered{};
	bool released{};
};

struct AnalogActionStatus {
	AnalogActionStatus(AnalogAction action) : action(action) {}
	AnalogAction action{};

	float x{};
	float y{};
};

class ControllerMap {
  public:
	ControllerMap(automa::ServiceProvider& svc);

	ControllerMap(ControllerMap const&) = delete;
	ControllerMap operator=(ControllerMap const&) = delete;

	/// @brief Update internal action states. Call once on tick update, and nowhere else.
	void update();

	/// @brief Returns whether there is a gamepad connected or not.
	[[nodiscard]] auto gamepad_connected() const -> bool { return controller_handle != 0; }
	[[nodiscard]] auto autosprint() const -> bool { return hard_toggles.test(Toggles::autosprint); }
	[[nodiscard]] auto hard_toggles_off() const -> bool { return !hard_toggles.test(Toggles::keyboard) && !hard_toggles.test(Toggles::gamepad); }
	[[nodiscard]] auto digital_action_status(DigitalAction action) const -> DigitalActionStatus { return digital_actions.at(action).status; }
	[[nodiscard]] auto analog_action_status(AnalogAction action) const -> AnalogActionStatus { return analog_actions.at(action).second; }
	[[nodiscard]] auto digital_action_name(DigitalAction action) const -> std::string_view;
	/// @brief Set the current action set.
	/// @warning This determines the actions capable to be received by the connected gamepads, so remember to set it correctly!
	void set_action_set(ActionSet set);
	/// @brief Open the Steam controller configuration overlay.
	void open_bindings_overlay();

	void set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Key key);
	void set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Key key);
	/// @brief Returns the primary keyboard key associated with a particular action.
	/// @param action
	/// @return The key bound, or sf::Keyboard::Key::Unknown if no key was bound.
	[[nodiscard]] auto get_primary_keyboard_binding(DigitalAction action) -> sf::Keyboard::Key;
	/// @brief Returns the secondary keyboard key associated with a particular action.
	/// @param action
	/// @return The key bound, or sf::Keyboard::Key::Unknown if no key was bound.
	[[nodiscard]] auto get_secondary_keyboard_binding(DigitalAction action) -> sf::Keyboard::Key;

	[[nodiscard]] auto key_to_string(sf::Keyboard::Key) -> std::string_view;
	[[nodiscard]] auto string_to_key(std::string_view) -> sf::Keyboard::Key;

	std::unordered_map<int, std::string_view> gamepad_button_name{};

	std::unordered_map<std::string_view, sf::Mouse::Button> string_to_mousebutton{{"LMB", sf::Mouse::Left}, {"RMB", sf::Mouse::Right}};

	ControllerType type{};
	util::BitFlags<Toggles> hard_toggles{};
	util::BitFlags<ControllerStatus> status{};

  private:
	struct DigitalActionData {
		InputDigitalActionHandle_t steam_handle;
		DigitalActionStatus status;
		sf::Keyboard::Key primary_binding;
		sf::Keyboard::Key secondary_binding;
	};
	std::unordered_map<DigitalAction, DigitalActionData> digital_actions{};
	std::unordered_map<AnalogAction, std::pair<InputAnalogActionHandle_t, AnalogActionStatus>> analog_actions{};
	std::unordered_map<ActionSet, InputActionSetHandle_t> action_sets{};

	InputHandle_t controller_handle{};

	STEAM_CALLBACK(ControllerMap, handle_gamepad_connection, SteamInputDeviceConnected_t);

	STEAM_CALLBACK(ControllerMap, handle_gamepad_disconnection, SteamInputDeviceDisconnected_t);
};

} // namespace config
