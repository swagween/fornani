
#pragma once

#include <steam/isteaminput.h>
#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/systems/InputActionMap.hpp>
#include <array>
#include <unordered_map>
#include <unordered_set>

namespace fornani::input {

enum class AnalogAction : int { platformer_movement, map_movement, END };
enum class ActionSet { Platformer, Inventory, Map, Menu, END };
enum class InputDevice { none, keyboard, gamepad };

// raw input
struct RawDigitalState {
	bool held = false;	 // button/key is down
	bool active = false; // valid in current action set
};

struct RawAnalogState {
	float x = 0.f;
	float y = 0.f;
	bool active = false;
};

// processed input
struct ResolvedDigitalState {
	bool triggered = false; // pressed this frame
	bool held = false;		// currently held
	bool released = false;	// released this frame
};

struct ResolvedAnalogState {
	float x = 0.f;
	float y = 0.f;
};

// input storage unit
struct DigitalActionData {
	InputHandle_t steam_handle; // handle to SteamInput action
	sf::Keyboard::Scancode primary_binding;
	sf::Keyboard::Scancode secondary_binding;
};

// --------------------------------------------------
// Input system
// --------------------------------------------------

class InputSystem final {
  public:
	InputSystem(ResourceFinder& finder);
	void handle_event(std::optional<sf::Event> const event);
	void update(); // calls gather + resolve

	// --- Action set control ---
	void set_action_set(ActionSet set);
	[[nodiscard]] auto get_active_action_set() const -> ActionSet { return m_active_action_set; };

	// --- Gameplay queries ---
	auto digital(DigitalAction action) const -> ResolvedDigitalState const& { return m_resolved_digital[static_cast<size_t>(action)]; }
	auto analog(AnalogAction action) const -> ResolvedAnalogState const& { return m_resolved_analog[static_cast<size_t>(action)]; }

	[[nodiscard]] auto last_device_used() const -> InputDevice { return m_last_device_used; }

	// --- Joystic queries ---
	[[nodiscard]] auto get_joystick_throttle() const -> sf::Vector2f;
	[[nodiscard]] auto get_i_joystick_throttle(bool exclusive) const -> sf::Vector2i;
	void set_joystick_throttle(sf::Vector2f throttle);

  private:
	// ---- Phase 1 ----
	void gather_raw_input();

	// ---- Phase 2 ----
	void resolve_input();

	// ---- SteamInput helpers ----
	void update_steam_controllers();
	void activate_action_set_if_needed();
	void init_steam_action_sets();
	void setup_action_handles();
	void set_primary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);
	void set_secondary_keyboard_binding(DigitalAction action, sf::Keyboard::Scancode key);

	[[nodiscard]] auto get_primary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;
	[[nodiscard]] auto get_secondary_keyboard_binding(DigitalAction action) const -> sf::Keyboard::Scancode;
	[[nodiscard]] auto steam_handle_for(DigitalAction action) const -> InputHandle_t { return m_digital_actions.at(action).steam_handle; }
	[[nodiscard]] auto steam_handle_for(AnalogAction action) const -> InputHandle_t { return m_analog_actions.at(action).steam_handle; }

  private:
	// --- State ---
	ActionSet m_active_action_set{ActionSet::Platformer};
	InputDevice m_last_device_used{InputDevice::none};
	std::unordered_map<DigitalAction, DigitalActionData> m_digital_actions{};
	std::unordered_map<AnalogAction, DigitalActionData> m_analog_actions{};

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
};

} // namespace fornani::input
