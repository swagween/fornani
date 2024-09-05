
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
	toggleables.gamepad = options.at(2).label;
	music_label = options.at(3).label;
	options.at(0).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(1).label.setString(toggleables.tutorial.getString() + (svc.controller_map.is_tutorial_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(2).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(3).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");
}

void SettingsMenu::init(ServiceProvider& svc, int room_number) {}

void SettingsMenu::handle_events(ServiceProvider& svc, sf::Event& event) {}

void SettingsMenu::tick_update(ServiceProvider& svc) {
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		current_selection.modulate(1);
		if (mode_flags.test(MenuMode::adjust)) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
		mode_flags.reset(MenuMode::adjust);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		current_selection.modulate(-1);
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
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			svc.data.save_settings();
		}
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered && !adjust_mode()) {
		svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
		switch (current_selection.get()) {
		case static_cast<int>(Toggles::autosprint): svc.controller_map.enable_autosprint(!svc.controller_map.is_autosprint_enabled()); break;
		case static_cast<int>(Toggles::tutorial): svc.controller_map.enable_tutorial(!svc.controller_map.is_tutorial_enabled()); break;
		case static_cast<int>(Toggles::gamepad): svc.controller_map.enable_gamepad_input(!svc.controller_map.is_gamepad_input_enabled()); break;
		case static_cast<int>(Toggles::music): adjust_mode() ? mode_flags.reset(MenuMode::adjust) : mode_flags.set(MenuMode::adjust); break;
		}
		options.at(static_cast<int>(Toggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(static_cast<int>(Toggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.controller_map.is_tutorial_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		options.at(static_cast<int>(Toggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	}
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);
	for (auto& option : options) {
		option.update(svc, current_selection.get());
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
	auto index = static_cast<int>(Toggles::music);

	adjust_mode() ? options.at(index).label.setFillColor(svc.styles.colors.red) : options.at(index).label.setFillColor(options.at(index).label.getFillColor());
	options.at(index).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music.volume.multiplier * 100.f)) + "%");

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
