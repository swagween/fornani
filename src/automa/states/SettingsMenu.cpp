
#include "SettingsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
	toggle_options.enabled.setString("Enabled");
	toggle_options.disabled.setString("Disabled");

	toggleables.keyboard = options.at(0).label;
	toggleables.gamepad = options.at(1).label;
	options.at(0).label.setString(toggleables.keyboard.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::keyboard) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(1).label.setString(toggleables.gamepad.getString() +(svc.controller_map.hard_toggles.test(config::Toggles::gamepad) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
}

void SettingsMenu::init(ServiceProvider& svc, std::string_view room) {}

void SettingsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

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
	if (svc.controller_map.label_to_control.at("left").triggered()) {
		svc.state_controller.submenu = menu_type::options;
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.label_to_control.at("main_action").triggered()) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		switch (current_selection) {
		case 0:
			if (svc.controller_map.hard_toggles.test(config::Toggles::keyboard)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::keyboard);
				if (svc.controller_map.hard_toggles_off()) { svc.controller_map.hard_toggles.set(config::Toggles::keyboard); }
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::keyboard);
			}
			break;
		case 1:
			if (svc.controller_map.hard_toggles.test(config::Toggles::gamepad)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::gamepad);
				if (svc.controller_map.hard_toggles_off()) { svc.controller_map.hard_toggles.set(config::Toggles::gamepad); }
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::gamepad);
			}
			break;
		}
		if (!svc.controller_map.gamepad_connected()) { svc.controller_map.hard_toggles.set(config::Toggles::keyboard); }
		options.at(0).label.setString(toggleables.keyboard.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::keyboard) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(1).label.setString(toggleables.gamepad.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::gamepad) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	}
	if (svc.controller_map.label_to_control.at("right").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_back").triggered()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	svc.controller_map.reset_triggers();
}

void SettingsMenu::tick_update(ServiceProvider& svc) {for (auto& option : options) { option.update(svc, current_selection); }
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);

	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
