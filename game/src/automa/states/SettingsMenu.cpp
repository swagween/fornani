
#include "fornani/automa/states/SettingsMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player)
	: MenuState(svc, player, "settings"), toggleables{.autosprint = options.at(static_cast<int>(SettingsToggles::autosprint)).label,
													  .tutorial = options.at(static_cast<int>(SettingsToggles::tutorial)).label,
													  .gamepad = options.at(static_cast<int>(SettingsToggles::gamepad)).label,
													  .fullscreen = options.at(static_cast<int>(SettingsToggles::fullscreen)).label,
													  .military_time = options.at(static_cast<int>(SettingsToggles::military_time)).label},
	  music_label{options.at(static_cast<int>(SettingsToggles::music)).label}, ambience_label{options.at(static_cast<int>(SettingsToggles::ambience)).label}, sfx_label{options.at(static_cast<int>(SettingsToggles::sfx)).label},
	  toggle_options{.enabled{svc.text.fonts.title}, .disabled{svc.text.fonts.title}}, sliders{.music_volume{svc.text.fonts.title}, .ambience_volume{svc.text.fonts.title}, .sfx_volume{svc.text.fonts.title}} {
	m_parent_menu = MenuType::options;
	toggle_options.enabled.setString("enabled");
	toggle_options.disabled.setString("disabled");

	options.at(static_cast<int>(SettingsToggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.input_system.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.input_system.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::music)).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.get_volume() * 100.f)) + "%");
	options.at(static_cast<int>(SettingsToggles::ambience)).label.setString(ambience_label.getString() + std::to_string(static_cast<int>(svc.ambience_player.get_volume() * 100.f)) + "%");
	options.at(static_cast<int>(SettingsToggles::sfx)).label.setString(sfx_label.getString() + std::to_string(static_cast<int>(svc.soundboard.get_volume() * 100.f)) + "%");
	options.at(static_cast<int>(SettingsToggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
}

void SettingsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	m_input_authorized = !adjust_mode() && !m_console;
	adjust_mode() ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);
	if (!m_console) {
		if (svc.input_system.menu_move(input::MoveDirection::down)) {
			if (adjust_mode()) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
			m_mode = SettingsMenuMode::ready;
		}
		if (svc.input_system.menu_move(input::MoveDirection::up)) {
			if (adjust_mode()) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
			m_mode = SettingsMenuMode::ready;
		}
		if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
			if (adjust_mode()) {
				m_mode = SettingsMenuMode::ready;
				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			} else {
				svc.data.save_settings();
			}
		}
		if (svc.input_system.digital(input::DigitalAction::menu_select).triggered && !adjust_mode()) {
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			switch (current_selection.get()) {
			case static_cast<int>(SettingsToggles::autosprint): svc.input_system.set_flag(input::InputSystemFlags::auto_sprint, !svc.input_system.is_autosprint_enabled()); break;
			case static_cast<int>(SettingsToggles::tutorial): svc.toggle_tutorial(); break;
			case static_cast<int>(SettingsToggles::gamepad): svc.input_system.set_flag(input::InputSystemFlags::gamepad_input_enabled, !svc.input_system.is_gamepad_input_enabled()); break;
			case static_cast<int>(SettingsToggles::music): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::ambience): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::sfx): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::fullscreen):
				svc.toggle_fullscreen();
				m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "fullscreen", gui::OutputType::gradual);
				break;
			case static_cast<int>(SettingsToggles::military_time): svc.world_clock.toggle_military_time(); break;
			}
			options.at(static_cast<int>(SettingsToggles::autosprint))
				.label.setString(toggleables.autosprint.getString() + (svc.input_system.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.input_system.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
		} else if (svc.input_system.digital(input::DigitalAction::menu_select).triggered && adjust_mode()) {
			m_mode = SettingsMenuMode::ready;
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
	}
	MenuState::tick_update(svc, engine);
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setLetterSpacing(1.2f);
	}
	if (adjust_mode()) {
		auto const update_volume = svc.ticker.every_x_ticks(16);
		auto delta = 0.01f;
		auto left = svc.input_system.menu_move(input::MoveDirection::left, input::DigitalActionQueryType::held);
		auto right = svc.input_system.menu_move(input::MoveDirection::right, input::DigitalActionQueryType::held);
		if (is(SettingsToggles::music)) {
			if (left && update_volume) { svc.music_player.adjust_volume(-delta); }
			if (right && update_volume) { svc.music_player.adjust_volume(delta); }
		}
		if (is(SettingsToggles::ambience)) {
			if (left && update_volume) { svc.ambience_player.adjust_volume(-delta); }
			if (right && update_volume) { svc.ambience_player.adjust_volume(delta); }
		}
		if (is(SettingsToggles::sfx)) {
			if (left && update_volume) { svc.soundboard.adjust_volume(-delta); }
			if (right && update_volume) { svc.soundboard.adjust_volume(delta); }
		}
	}
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	auto index = is(SettingsToggles::music) ? static_cast<int>(SettingsToggles::music) : is(SettingsToggles::ambience) ? static_cast<int>(SettingsToggles::ambience) : static_cast<int>(SettingsToggles::sfx);

	adjust_mode() ? options.at(index).label.setFillColor(p_theme.activated_text_color) : options.at(index).label.setFillColor(options.at(index).label.getFillColor());
	MenuState::render(svc, win);
	if (is(SettingsToggles::music)) { options.at(index).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.get_volume() * 100.f)) + "%"); }
	if (is(SettingsToggles::ambience)) { options.at(index).label.setString(ambience_label.getString() + std::to_string(static_cast<int>(svc.ambience_player.get_volume() * 100.f)) + "%"); }
	if (is(SettingsToggles::sfx)) { options.at(index).label.setString(sfx_label.getString() + std::to_string(static_cast<int>(svc.soundboard.get_volume() * 100.f)) + "%"); }

	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win, true);
	}
}

} // namespace fornani::automa
