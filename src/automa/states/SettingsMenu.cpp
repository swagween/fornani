
#include "SettingsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
	toggle_options.enabled.setString("Enabled");
	toggle_options.disabled.setString("Disabled");

	toggleables.autosprint = options.at(0).label;
	toggleables.tutorial = options.at(1).label;
	toggleables.keyboard = options.at(2).label;
	toggleables.gamepad = options.at(3).label;
	music_label = options.at(4).label;
	options.at(0).label.setString(toggleables.autosprint.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::autosprint) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(1).label.setString(toggleables.tutorial.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::tutorial) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(2).label.setString(toggleables.keyboard.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::keyboard) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(3).label.setString(toggleables.gamepad.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::gamepad) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(4).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");
}

void SettingsMenu::init(ServiceProvider& svc, int room_number) {}

void SettingsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {
	svc.controller_map.handle_mouse_events(event);
	svc.controller_map.handle_joystick_events(event);
	if (event.type == sf::Event::EventType::KeyPressed) { svc.controller_map.handle_press(event.key.code); }
	if (event.type == sf::Event::EventType::KeyReleased) { svc.controller_map.handle_release(event.key.code); }

	if (svc.controller_map.label_to_control.at("down").triggered()) {
		current_selection.modulate(1);
		mode_flags.reset(MenuMode::adjust);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("up").triggered()) {
		current_selection.modulate(-1);
		mode_flags.reset(MenuMode::adjust);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.label_to_control.at("left").triggered()) {
		if (adjust_mode()) {
		} else {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::exit_submenu);
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			svc.data.save_settings();
		}
	}
	if (svc.controller_map.label_to_control.at("main_action").triggered() && !adjust_mode()) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		switch (current_selection.get()) {
		case 0:
			if (svc.controller_map.hard_toggles.test(config::Toggles::autosprint)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::autosprint);
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::autosprint);
			}
			break;
		case 1:
			if (svc.controller_map.hard_toggles.test(config::Toggles::tutorial)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::tutorial);
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::tutorial);
			}
			break;
		case 2:
			if (svc.controller_map.hard_toggles.test(config::Toggles::keyboard)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::keyboard);
				if (svc.controller_map.hard_toggles_off()) { svc.controller_map.hard_toggles.set(config::Toggles::keyboard); }
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::keyboard);
			}
			break;
		case 3:
			if (svc.controller_map.hard_toggles.test(config::Toggles::gamepad)) {
				svc.controller_map.hard_toggles.reset(config::Toggles::gamepad);
				if (svc.controller_map.hard_toggles_off()) { svc.controller_map.hard_toggles.set(config::Toggles::gamepad); }
			} else {
				svc.controller_map.hard_toggles.set(config::Toggles::gamepad);
			}
			break;
		case 4: adjust_mode() ? mode_flags.reset(MenuMode::adjust) : mode_flags.set(MenuMode::adjust);
			break;
		}
		if (!svc.controller_map.gamepad_connected()) { svc.controller_map.hard_toggles.set(config::Toggles::keyboard); }
		options.at(0).label.setString(toggleables.autosprint.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::autosprint) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(1).label.setString(toggleables.keyboard.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::keyboard) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(2).label.setString(toggleables.gamepad.getString() + (svc.controller_map.hard_toggles.test(config::Toggles::gamepad) ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	}
	if (svc.controller_map.label_to_control.at("right").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_forward").triggered()) {}
	if (svc.controller_map.label_to_control.at("menu_back").triggered() || svc.controller_map.label_to_control.at("menu_toggle_secondary").triggered()) {
		svc.state_controller.actions.set(Actions::exit_submenu);
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		svc.data.save_settings();
	}
	svc.controller_map.reset_triggers();
}

void SettingsMenu::tick_update(ServiceProvider& svc) {
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		option.label.setLetterSpacing(1.2f);
	}
	if (svc.ticker.every_x_ticks(16)) {
		if (svc.controller_map.label_to_control.at("left").held() && adjust_mode()) { svc.music.volume.multiplier = std::clamp(svc.music.volume.multiplier - 0.01f, 0.f, 1.f); }
		if (svc.controller_map.label_to_control.at("right").held() && adjust_mode()) { svc.music.volume.multiplier = std::clamp(svc.music.volume.multiplier + 0.01f, 0.f, 1.f); }
	}
	svc.soundboard.play_sounds(svc);
	svc.controller_map.reset_triggers();
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	adjust_mode() ? options.at(3).label.setFillColor(svc.styles.colors.red) : options.at(3).label.setFillColor(options.at(3).label.getFillColor());
	options.at(3).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
