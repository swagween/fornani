
#include "SettingsMenu.hpp"
#include "../../service/ServiceProvider.hpp"

namespace automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int id) : GameState(svc, player, scene, id) {
	left_dot.set_position(options.at(current_selection).left_offset);
	right_dot.set_position(options.at(current_selection).right_offset);
	toggle_options.enabled.setString("Enabled");
	toggle_options.disabled.setString("Disabled");

	toggleables.autosprint = options.at(0).label;
	toggleables.gamepad = options.at(1).label;
	music_label = options.at(2).label;
	options.at(0).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(1).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(2).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");
}

void SettingsMenu::init(ServiceProvider& svc, int room_number) {}

void SettingsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {}

void SettingsMenu::tick_update(ServiceProvider& svc) {
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		++current_selection;
		constrain_selection();
		if (mode_flags.test(MenuMode::adjust)) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
		mode_flags.reset(MenuMode::adjust);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		--current_selection;
		constrain_selection();
		if (mode_flags.test(MenuMode::adjust)) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
		mode_flags.reset(MenuMode::adjust);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		if (adjust_mode()) {
			mode_flags.reset(MenuMode::adjust);
		} else {
			svc.state_controller.submenu = menu_type::options;
			svc.state_controller.actions.set(Actions::exit_submenu);
		}
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered && !adjust_mode()) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		switch (current_selection) {
		case 0: svc.controller_map.enable_autosprint(!svc.controller_map.is_autosprint_enabled()); break;
		case 1: svc.controller_map.enable_gamepad_input(!svc.controller_map.is_gamepad_input_enabled()); break;
		case 2: adjust_mode() ? mode_flags.reset(MenuMode::adjust) : mode_flags.set(MenuMode::adjust); break;
		}
		options.at(0).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(1).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	}
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection).left_offset);
	right_dot.set_target_position(options.at(current_selection).right_offset);
	for (auto& option : options) {
		option.update(svc, current_selection);
		option.label.setLetterSpacing(1.2f);
	}
	if (svc.ticker.every_x_ticks(16)) {
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).held && adjust_mode()) { svc.music.volume.multiplier = std::clamp(svc.music.volume.multiplier - 0.01f, 0.f, 1.f); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).held && adjust_mode()) { svc.music.volume.multiplier = std::clamp(svc.music.volume.multiplier + 0.01f, 0.f, 1.f); }
	}
	svc.soundboard.play_sounds(svc);
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {

	adjust_mode() ? options.at(2).label.setFillColor(svc.styles.colors.red) : options.at(2).label.setFillColor(options.at(2).label.getFillColor());
	options.at(2).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");
	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
