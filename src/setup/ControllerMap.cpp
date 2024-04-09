#include "ControllerMap.hpp"
#include <iostream>
#include "../service/ServiceProvider.hpp"

namespace config {

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {
	label_to_control.insert({"main_action", Control(Action::main_action)});
	label_to_control.insert({"secondary_action", Control(Action::secondary_action)});
	label_to_control.insert({"tertiary_action", Control(Action::tertiary_action)});
	label_to_control.insert({"inspect", Control(Action::inspect)});
	label_to_control.insert({"menu_toggle", Control(Action::menu_toggle)});
	label_to_control.insert({"menu_toggle_secondary", Control(Action::menu_toggle)});
	label_to_control.insert({"arms_switch_left", Control(Action::arms_switch_left)});
	label_to_control.insert({"arms_switch_right", Control(Action::arms_switch_right)});
	label_to_control.insert({"left", Control(Action::left)});
	label_to_control.insert({"right", Control(Action::right)});
	label_to_control.insert({"up", Control(Action::up)});
	label_to_control.insert({"down", Control(Action::down)});

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
}

void ControllerMap::handle_mouse_events(sf::Event& event) {
	if (event.type == sf::Event::EventType::MouseButtonPressed && mousebutton_to_label.contains(event.mouseButton.button)) { label_to_control.at(mousebutton_to_label.at(event.mouseButton.button)).press(); }
	if (event.type == sf::Event::EventType::MouseButtonReleased && mousebutton_to_label.contains(event.mouseButton.button)) { label_to_control.at(mousebutton_to_label.at(event.mouseButton.button)).release(); }
}

void ControllerMap::handle_press(sf::Keyboard::Key& k) {
	if (key_to_label.contains(k)) { label_to_control.at(key_to_label.at(k)).press(); }
}

void ControllerMap::handle_release(sf::Keyboard::Key& k) {
	if (key_to_label.contains(k)) { label_to_control.at(key_to_label.at(k)).release(); }
}

void ControllerMap::handle_joystick_events(sf::Event& event) {
	if (type != ControllerType::gamepad) { return; }
	// left analog stick
	throttle.x = sf::Joystick::getAxisPosition(0, sf::Joystick::X) / 100.f;
	if (abs(throttle.x) < throttle_threshold) { throttle.x = 0.f; }
	throttle.y = sf::Joystick::getAxisPosition(0, sf::Joystick::Y) / 100.f;
	if (abs(throttle.y) < throttle_threshold) { throttle.y = 0.f; }

	if (event.type == sf::Event::EventType::JoystickMoved) {
		if (throttle.x < -throttle_threshold && !label_to_control.at("left").held()) { label_to_control.at("left").press(); }
		if (throttle.x > throttle_threshold && !label_to_control.at("right").held()) { label_to_control.at("right").press(); }
		if (throttle.y < -throttle_threshold && !label_to_control.at("up").held()) { label_to_control.at("up").press(); }
		if (throttle.y > throttle_threshold && !label_to_control.at("down").held()) { label_to_control.at("down").press(); }

		if (throttle.x > -throttle_threshold && label_to_control.at("left").held()) { label_to_control.at("left").release(); }
		if (throttle.x < throttle_threshold && label_to_control.at("right").held()) { label_to_control.at("right").release(); }
		if (throttle.y > -throttle_threshold && label_to_control.at("up").held()) { label_to_control.at("up").release(); }
		if (throttle.y < throttle_threshold && label_to_control.at("down").held()) { label_to_control.at("down").release(); }
	}

	// gamepad buttons
	for (auto& tag : tags) {
		if (!label_to_gamepad.contains(tag)) { continue; }
		if (!label_to_control.contains(tag)) { continue; }
		if (event.type == sf::Event::JoystickButtonPressed) {
			if (sf::Joystick::isButtonPressed(0, label_to_gamepad.at(tag)) && !label_to_control.at(tag).held()) { label_to_control.at(tag).press(); }
		} else if (event.type == sf::Event::JoystickButtonReleased) {
			if (!sf::Joystick::isButtonPressed(0, label_to_gamepad.at(tag)) && label_to_control.at(tag).held()) { label_to_control.at(tag).release(); }
		}
	}
}

void ControllerMap::reset_triggers() {
	for (auto& tag : tags) { label_to_control.at(tag).reset_triggers(); }
}

void ControllerMap::switch_to_joystick() {}

void ControllerMap::switch_to_keyboard() {}

} // namespace config