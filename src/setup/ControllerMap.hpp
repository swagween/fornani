
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace config {

	enum class Action { left, right, up, down, main_action, tertiary_action, secondary_action, inspect, arms_switch_left, arms_switch_right, menu_toggle, menu_toggle_secondary };
	enum class ActionState { held, released, triggered };
	enum class ControllerType { keyboard, gamepad };

	struct Control {
		Control(Action action) : action(action) {}
		Action action{};
		util::BitFlags<ActionState> state{};
		[[nodiscard]] auto held() const -> bool { return state.test(ActionState::held); }
		[[nodiscard]] auto released() const -> bool { return state.test(ActionState::released); }
		[[nodiscard]] auto triggered() const -> bool { return state.test(ActionState::triggered); }
		void press() {
			state.set(ActionState::triggered);
			state.set(ActionState::held);
		}
		void release() {
			state.set(ActionState::released);
			state.reset(ActionState::held);
		}
		void reset_triggers() {
			state.reset(ActionState::triggered);
			state.reset(ActionState::released);
		}
	};

class ControllerMap {
  public:
	ControllerMap(automa::ServiceProvider& svc);
	void handle_press(sf::Keyboard::Key& k);
	void handle_release(sf::Keyboard::Key& k);
	void handle_joystick_events(sf::Event& event);
	void reset_triggers();
	[[nodiscard]] auto get_throttle() const -> sf::Vector2<float> { return throttle; }

	std::unordered_map<std::string_view, Control> label_to_control{};
	std::unordered_map<sf::Keyboard::Key, std::string_view> key_to_label{};
	std::unordered_map<std::string_view, sf::Keyboard::Key> string_to_key { {"Z", sf::Keyboard::Z} };

	ControllerType type{};

  private:
	sf::Vector2<float> throttle{};
	float const throttle_threshold{0.4f};
};

} // namespace lookup
