
#include "ControlsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

ControlsMenu::ControlsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	int ctr{0};
	for (auto& option : options) {
		option.position.x = svc.constants.screen_dimensions.x * 0.5f - center_offset;
		option.update(svc, current_selection);
		option.left_offset = option.position - sf::Vector2<float>{option.dot_offset.x - 2, -option.dot_offset.y};
		option.right_offset = option.position + sf::Vector2<float>{option.label.getLocalBounds().width + option.dot_offset.x, option.dot_offset.y};
		option.label.setLetterSpacing(title_letter_spacing);
		control_list.push_back(sf::Text());
		control_list.back().setFont(font);
		control_list.back().setLetterSpacing(title_letter_spacing);
		++ctr;
	}
	refresh_controls(svc);
	instruction.setLineSpacing(1.5f);
	instruction.setFont(font);
	instruction.setLetterSpacing(title_letter_spacing);
	instruction.setCharacterSize(options.at(current_selection).label.getCharacterSize());
	instruction.setPosition(svc.constants.screen_dimensions.x * 0.5f - instruction.getLocalBounds().width * 0.5f, svc.constants.screen_dimensions.y - 120.f);
	instruction.setFillColor(svc.styles.colors.dark_grey);

	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
	loading.start(2);

	debug.setFillColor(sf::Color::Transparent);
	debug.setSize({2.f, (float)svc.constants.screen_dimensions.y});
	debug.setOutlineColor(svc.styles.colors.blue);
	debug.setOutlineThickness(-1);
}

void ControlsMenu::init(ServiceProvider& svc, int room_number) {}

void ControlsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {

	if (event.type == sf::Event::EventType::KeyPressed && event.key.code == sf::Keyboard::Escape) { binding_mode = false; }
	if (binding_mode) {
		update_binding(svc, event);
		return;
	}
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) {
		if (event.key.code == sf::Keyboard::Enter && svc.controller_map.is_keyboard()) { binding_mode = true; }
		if (event.key.code == sf::Keyboard::LControl || event.key.code == sf::Keyboard::RControl) { restore_defaults(svc); }
		svc.controller_map.handle_press(event.key.code);
	}
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (event.type == sf::Event::EventType::JoystickButtonPressed) {
		if (event.joystickButton.button == 9) { binding_mode = true; }
	}

	if (svc.controller_map.label_to_control.at("down").triggered()) {
		++current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		--current_selection;
		constrain_selection();
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("left").triggered() && !svc.controller_map.is_gamepad()) {
		svc.state_controller.submenu = menu_type::options;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("right").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_back").triggered()) {
		svc.state_controller.submenu = menu_type::options;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}

	if (event.type == sf::Event::EventType::JoystickDisconnected) { refresh_controls(svc); }
	if (event.type == sf::Event::EventType::JoystickConnected) { refresh_controls(svc); }
	if (event.type == sf::Event::JoystickButtonPressed || svc.controller_map.joystick_moved()) { refresh_controls(svc); }
	if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) { refresh_controls(svc); }

	svc.controller_map.reset_triggers();
}

void ControlsMenu::tick_update(ServiceProvider& svc) {
	if (binding_mode) { return; }
	int ctr{0};
	for (auto& option : options) {
		option.update(svc, current_selection);
		option.label.setOrigin(0, option.label.getLocalBounds().height * 0.5f);
		option.left_offset = option.position - sf::Vector2<float>{option.dot_offset.x - 2, -option.dot_offset.y};
		option.right_offset = option.position + sf::Vector2<float>{option.label.getLocalBounds().width + option.dot_offset.x, option.dot_offset.y};
	
		control_list.at(ctr).setFillColor(option.label.getFillColor());
		++ctr;
	}
	loading.update();
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void ControlsMenu::frame_update(ServiceProvider& svc) {}

void ControlsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	if (loading.is_complete()) {
		for (auto& option : options) { win.draw(option.label); }
		for (auto& control : control_list) { win.draw(control); }
		win.draw(instruction);
	}

	left_dot.render(svc, win, {});
	right_dot.render(svc, win, {});
}

void ControlsMenu::refresh_controls(ServiceProvider& svc) {
	svc.data.load_controls(svc.controller_map);
	size_t ctr{0};

	for (auto& option : options) {
		if (ctr >= control_list.size() || ctr >= svc.controller_map.tags.size()) { continue; }
		control_list.at(ctr).setString(svc.controller_map.tag_to_label.at(svc.controller_map.tags.at(ctr)).data());
		control_list.at(ctr).setCharacterSize(option.label.getCharacterSize());
		control_list.at(ctr).setOrigin(control_list.at(ctr).getLocalBounds().width, control_list.at(ctr).getLocalBounds().height * 0.5f);
		control_list.at(ctr).setPosition(svc.constants.screen_dimensions.x * 0.5f + center_offset, option.position.y);
		++ctr;
	}
	ctr = 0;
	// check for duplicate controls
	for (auto& comparison : control_list) {
		int dup_check{};
		for (auto& this_one : control_list) {
			if (this_one.getString() == "left analog stick") { continue; }
			if (this_one.getString() == comparison.getString()) { ++dup_check; }
		}
		options.at(ctr).flagged = dup_check > 1;
		++ctr;
	}
	svc.controller_map.is_gamepad() ? instruction.setString("current controller : GAMEPAD\npress [START] to change binding, and [Esc] to cancel\npress [Ctrl] to restore defaults")
									: instruction.setString("current controller : KEYBOARD\npress [ENTER] to change binding, and [Esc] to cancel\npress [Ctrl] to restore defaults");
}

void ControlsMenu::update_binding(ServiceProvider& svc, sf::Event& event) {
	if (current_selection >= options.size() || current_selection >= control_list.size() || current_selection >= svc.controller_map.tags.size()) { return; }
	options.at(current_selection).label.setFillColor(svc.styles.colors.bright_orange);
	control_list.at(current_selection).setFillColor(svc.styles.colors.bright_orange);
	if (svc.controller_map.is_keyboard()) {
		if (event.type == sf::Event::KeyPressed) {
			binding_mode = false;
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["mouse_button"] = "";
			if (!svc.controller_map.key_to_string.contains(event.key.code)) { return; }
			svc.data.controls["controls"][tag]["keyboard_key"] = svc.controller_map.key_to_string.at(event.key.code);
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc);
		}
		if (event.type == sf::Event::MouseButtonPressed) {
			binding_mode = false;
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["keyboard_key"] = "";
			if (event.mouseButton.button == sf::Mouse::Left) { svc.data.controls["controls"][tag]["mouse_button"] = "LMB"; }
			if (event.mouseButton.button == sf::Mouse::Right) { svc.data.controls["controls"][tag]["mouse_button"] = "RMB"; }
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc);
		}
	}
	if (svc.controller_map.is_gamepad()) {
		if (event.type == sf::Event::JoystickButtonPressed) {
			binding_mode = false;
			std::string_view tag = svc.controller_map.tags.at(current_selection);
			svc.data.controls["controls"][tag]["gamepad_button"] = event.joystickButton.button;
			svc.data.save_controls(svc.controller_map);
			refresh_controls(svc);
		}
	}
}

void ControlsMenu::restore_defaults(ServiceProvider& svc) {
	svc.data.reset_controls();
	svc.data.save_controls(svc.controller_map);
	svc.data.load_controls(svc.controller_map);
	refresh_controls(svc);
}

} // namespace automa
