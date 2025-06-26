
#include "fornani/automa/states/SettingsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number)
	: GameState(svc, player, scene, room_number), toggleables{.autosprint = options.at(static_cast<int>(Toggles::autosprint)).label,
															  .tutorial = options.at(static_cast<int>(Toggles::tutorial)).label,
															  .gamepad = options.at(static_cast<int>(Toggles::gamepad)).label,
															  .fullscreen = options.at(static_cast<int>(Toggles::fullscreen)).label,
															  .military_time = options.at(static_cast<int>(Toggles::military_time)).label},
	  music_label{options.at(static_cast<int>(Toggles::music)).label}, toggle_options{.enabled{svc.text.fonts.title}, .disabled{svc.text.fonts.title}}, sliders{.music_volume{svc.text.fonts.title}} {
	left_dot.set_position(options.at(current_selection.get()).left_offset);
	right_dot.set_position(options.at(current_selection.get()).right_offset);
	toggle_options.enabled.setString("enabled");
	toggle_options.disabled.setString("disabled");

	options.at(static_cast<int>(Toggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(Toggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(Toggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(Toggles::music)).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.get_volume_multiplier() * 100.f)) + "%");
	options.at(static_cast<int>(Toggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(Toggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
}

void SettingsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);
	adjust_mode() ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);
	svc.controller_map.set_action_set(config::ActionSet::Menu);
	if (!m_console) {
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
			current_selection.modulate(1);
			if (adjust_mode()) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
			m_mode = MenuMode::ready;
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
			current_selection.modulate(-1);
			if (adjust_mode()) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
			m_mode = MenuMode::ready;
			svc.soundboard.flags.menu.set(audio::Menu::shift);
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
			if (adjust_mode()) {
				m_mode = MenuMode::ready;
			} else {
				svc.state_controller.submenu = MenuType::options;
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
			case static_cast<int>(Toggles::tutorial): svc.toggle_tutorial(); break;
			case static_cast<int>(Toggles::gamepad): svc.controller_map.enable_gamepad_input(!svc.controller_map.is_gamepad_input_enabled()); break;
			case static_cast<int>(Toggles::music): m_mode = adjust_mode() ? MenuMode::ready : MenuMode ::adjust; break;
			case static_cast<int>(Toggles::fullscreen):
				svc.toggle_fullscreen();
				m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "fullscreen", gui::OutputType::gradual);
				break;
			case static_cast<int>(Toggles::military_time): svc.world_clock.toggle_military_time(); break;
			}
			options.at(static_cast<int>(Toggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.controller_map.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(Toggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(Toggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.controller_map.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(Toggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(Toggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		}
	}

	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		option.label.setLetterSpacing(1.2f);
	}
	if (adjust_mode()) {
		auto const update_volume = svc.ticker.every_x_ticks(16);
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_left).held && update_volume) { svc.music_player.set_volume_multiplier(svc.music_player.get_volume_multiplier() - 0.01f); }
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_right).held && update_volume) { svc.music_player.set_volume_multiplier(svc.music_player.get_volume_multiplier() + 0.01f); }
	}
	if (m_console) { m_console.value()->update(svc); }
	svc.soundboard.play_sounds(engine, svc);
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	auto index = static_cast<int>(Toggles::music);

	adjust_mode() ? options.at(index).label.setFillColor(colors::red) : options.at(index).label.setFillColor(options.at(index).label.getFillColor());
	options.at(index).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.get_volume_multiplier() * 100.f)) + "%");

	for (auto& option : options) { win.draw(option.label); }

	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win, true);
	}
}

} // namespace fornani::automa
