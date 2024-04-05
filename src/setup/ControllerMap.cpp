#include "ControllerMap.hpp"
#include "../service/ServiceProvider.hpp"
#include <iostream>

namespace config {

ControllerMap::ControllerMap(automa::ServiceProvider& svc) {

	label_to_control.insert({"main_action", Control(Action::main_action)});
	label_to_control.insert({"secondary_action", Control(Action::secondary_action)});
	label_to_control.insert({"tertiary_action", Control(Action::tertiary_action)});
	label_to_control.insert({"inspect", Control(Action::inspect)});
	label_to_control.insert({"menu_toggle", Control(Action::menu_toggle)});
	label_to_control.insert({"arms_switch_left", Control(Action::arms_switch_left)});
	label_to_control.insert({"arms_switch_right", Control(Action::arms_switch_right)});
	label_to_control.insert({"left", Control(Action::left)});
	label_to_control.insert({"right", Control(Action::right)});
	label_to_control.insert({"up", Control(Action::up)});
	label_to_control.insert({"down", Control(Action::down)});

	// we should load this from a json later
	//key_to_label.insert({sf::Keyboard::Z, "main_action"});
	key_to_label.insert({sf::Keyboard::X, "secondary_action"});
	key_to_label.insert({sf::Keyboard::C, "tertiary_action"});
	key_to_label.insert({sf::Keyboard::LShift, "inspect"});
	key_to_label.insert({sf::Keyboard::E, "menu_toggle"});
	key_to_label.insert({sf::Keyboard::A, "arms_switch_left"});
	key_to_label.insert({sf::Keyboard::S, "arms_switch_right"});
	key_to_label.insert({sf::Keyboard::Left, "left"});
	key_to_label.insert({sf::Keyboard::Right, "right"});
	key_to_label.insert({sf::Keyboard::Up, "up"});
	key_to_label.insert({sf::Keyboard::Down, "down"});
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
	// also should pull from a json-loaded button map
	if (event.type == sf::Event::JoystickButtonPressed) {
		if (sf::Joystick::isButtonPressed(0, 1) && !label_to_control.at("main_action").held()) { label_to_control.at("main_action").press(); }
		if (sf::Joystick::isButtonPressed(0, 7) && !label_to_control.at("secondary_action").held()) { label_to_control.at("secondary_action").press(); }
		if (sf::Joystick::isButtonPressed(0, 2) && !label_to_control.at("inspect").held()) { label_to_control.at("inspect").press(); }
		if (sf::Joystick::isButtonPressed(0, 3) && !label_to_control.at("menu_toggle").held()) { label_to_control.at("menu_toggle").press(); }
		if (sf::Joystick::isButtonPressed(0, 4) && !label_to_control.at("arms_switch_left").held()) { label_to_control.at("arms_switch_left").press(); }
		if (sf::Joystick::isButtonPressed(0, 5) && !label_to_control.at("arms_switch_right").held()) { label_to_control.at("arms_switch_right").press(); }
		if (sf::Joystick::isButtonPressed(0, 0) && !label_to_control.at("tertiary_action").held()) { label_to_control.at("tertiary_action").press(); }
	} else if(event.type == sf::Event::JoystickButtonReleased) {
		if (!sf::Joystick::isButtonPressed(0, 1) && label_to_control.at("main_action").held()) { label_to_control.at("main_action").release(); }
		if (!sf::Joystick::isButtonPressed(0, 7) && label_to_control.at("secondary_action").held()) { label_to_control.at("secondary_action").release(); }
		if (!sf::Joystick::isButtonPressed(0, 2) && label_to_control.at("inspect").held()) { label_to_control.at("inspect").release(); }
		if (!sf::Joystick::isButtonPressed(0, 3) && label_to_control.at("menu_toggle").held()) { label_to_control.at("menu_toggle").release(); }
		if (!sf::Joystick::isButtonPressed(0, 4) && label_to_control.at("arms_switch_left").held()) { label_to_control.at("arms_switch_left").release(); }
		if (!sf::Joystick::isButtonPressed(0, 5) && label_to_control.at("arms_switch_right").held()) { label_to_control.at("arms_switch_right").release(); }
		if (!sf::Joystick::isButtonPressed(0, 0) && label_to_control.at("tertiary_action").held()) { label_to_control.at("tertiary_action").release(); }
	}
}

void ControllerMap::reset_triggers() {
	label_to_control.at("main_action").reset_triggers();
	label_to_control.at("secondary_action").reset_triggers();
	label_to_control.at("tertiary_action").reset_triggers();
	label_to_control.at("arms_switch_left").reset_triggers();
	label_to_control.at("arms_switch_right").reset_triggers();
	label_to_control.at("menu_toggle").reset_triggers();
	label_to_control.at("inspect").reset_triggers();
	label_to_control.at("left").reset_triggers();
	label_to_control.at("right").reset_triggers();
	label_to_control.at("up").reset_triggers();
	label_to_control.at("down").reset_triggers();
}

} // namespace config