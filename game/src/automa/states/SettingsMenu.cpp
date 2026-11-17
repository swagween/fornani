
#include "fornani/automa/states/SettingsMenu.hpp"
#include <fornani/setup/AppContext.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

constexpr auto slider_size_v = 200.f;

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, AppContext& ctx)
	: MenuState(svc, player, ctx, "settings"), toggleables{.autosprint = options.at(static_cast<int>(SettingsToggles::autosprint)).label,
														   .tutorial = options.at(static_cast<int>(SettingsToggles::tutorial)).label,
														   .gamepad = options.at(static_cast<int>(SettingsToggles::gamepad)).label,
														   .fullscreen = options.at(static_cast<int>(SettingsToggles::fullscreen)).label,
														   .military_time = options.at(static_cast<int>(SettingsToggles::military_time)).label,
														   .language = options.at(static_cast<int>(SettingsToggles::language)).label},
	  m_adjustment{80}, volume_settings{.master{.label{svc.text.fonts.title.font}, .slider{slider_size_v}},
										.music{.label{svc.text.fonts.title.font}, .slider{slider_size_v}},
										.ambience{.label{svc.text.fonts.title.font}, .slider{slider_size_v}},
										.sfx{.label{svc.text.fonts.title.font}, .slider{slider_size_v}}},
	  toggle_options{.enabled{svc.text.fonts.title.font}, .disabled{svc.text.fonts.title.font}} {

	auto padding = 0.f;
	for (auto [i, option] : std::views::enumerate(options)) {
		auto const extra_space = i == static_cast<int>(SettingsToggles::master) || i == static_cast<int>(SettingsToggles::music) || i == static_cast<int>(SettingsToggles::ambience) || i == static_cast<int>(SettingsToggles::sfx);
		option.position = {svc.window->i_screen_dimensions().x * 0.5f, top_buffer + i * spacing + padding};
		option.update(current_selection.get());
		if (extra_space) { padding += 20.f; }
	}

	m_parent_menu = MenuType::options;
	toggle_options.enabled.setString(svc.data.gui_text["settings"]["enabled"].as_string());
	toggle_options.disabled.setString(svc.data.gui_text["settings"]["disabled"].as_string());
	m_lang = ctx.localization.get_language_title();

	volume_settings.master.label.setString(options.at(static_cast<int>(SettingsToggles::master)).label.getString());
	volume_settings.music.label.setString(options.at(static_cast<int>(SettingsToggles::music)).label.getString());
	volume_settings.ambience.label.setString(options.at(static_cast<int>(SettingsToggles::ambience)).label.getString());
	volume_settings.sfx.label.setString(options.at(static_cast<int>(SettingsToggles::sfx)).label.getString());

	options.at(static_cast<int>(SettingsToggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.input_system.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.input_system.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::master)).label.setString(volume_settings.master.label.getString() + std::to_string(static_cast<int>(svc.mixer.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::music)).label.setString(volume_settings.music.label.getString() + std::to_string(static_cast<int>(svc.music_player.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::ambience)).label.setString(volume_settings.ambience.label.getString() + std::to_string(static_cast<int>(svc.ambience_player.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::sfx)).label.setString(volume_settings.sfx.label.getString() + std::to_string(static_cast<int>(svc.soundboard.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));

	auto const offset = 24.f;
	volume_settings.master.slider.set_position({svc.window->f_center_screen().x - slider_size_v * 0.5f, options.at(static_cast<int>(SettingsToggles::master)).label.getGlobalBounds().position.y + offset});
	volume_settings.music.slider.set_position({svc.window->f_center_screen().x - slider_size_v * 0.5f, options.at(static_cast<int>(SettingsToggles::music)).label.getGlobalBounds().position.y + offset});
	volume_settings.ambience.slider.set_position({svc.window->f_center_screen().x - slider_size_v * 0.5f, options.at(static_cast<int>(SettingsToggles::ambience)).label.getGlobalBounds().position.y + offset});
	volume_settings.sfx.slider.set_position({svc.window->f_center_screen().x - slider_size_v * 0.5f, options.at(static_cast<int>(SettingsToggles::sfx)).label.getGlobalBounds().position.y + offset});

	if (m_lang) { options.at(static_cast<int>(SettingsToggles::language)).label.setString(toggleables.language.getString() + *m_lang); }
}

void SettingsMenu::on_exit() {
	p_app_context->settings.save_user_settings(*p_services);
	p_services->a11y.set_action_ctx_bar_enabled(p_app_context->settings.get_json()["tutorial"].as_bool());
}

void SettingsMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	m_input_authorized = !adjust_mode() && !p_context.console && !m_menu;
	adjust_mode() ? flags.reset(GameStateFlags::ready) : flags.set(GameStateFlags::ready);
	m_adjustment.update();

	if (!p_context.console) {
		if (m_menu) {
			m_menu->handle_inputs(svc.input_system, svc.soundboard);
			if (m_menu->was_closed()) { m_menu.reset(); }
		}
		if (was_selected(svc.input_system, true)) {
			if (m_menu) {
				auto selected_lang = 0;
				for (auto const& lang : p_app_context->localization.get_available_languages()) {
					auto tag = p_app_context->localization.get_tag_from_index(lang.index);
					if (lang.index == m_menu->get_selection()) { svc.events.set_language_event.dispatch(*p_app_context, tag); }
				}
				m_menu.reset();
				svc.input_system.cancel_input(input::DigitalAction::menu_select);
				svc.input_system.cancel_mouse();
			}
		}
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
			}
		}
		if (was_selected(svc.input_system) && !adjust_mode()) {
			svc.soundboard.flags.menu.set(audio::Menu::forward_switch);
			switch (current_selection.get()) {
			case static_cast<int>(SettingsToggles::autosprint): svc.input_system.set_setting(input::InputSystemSettings::auto_sprint, !svc.input_system.is_autosprint_enabled()); break;
			case static_cast<int>(SettingsToggles::tutorial): svc.toggle_tutorial(); break;
			case static_cast<int>(SettingsToggles::gamepad): svc.input_system.set_setting(input::InputSystemSettings::gamepad_input_enabled, !svc.input_system.is_gamepad_input_enabled()); break;
			case static_cast<int>(SettingsToggles::master): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::music): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::ambience): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::sfx): m_mode = adjust_mode() ? SettingsMenuMode::ready : SettingsMenuMode ::adjust; break;
			case static_cast<int>(SettingsToggles::fullscreen): {
				svc.window->recreate(!svc.window->is_fullscreen());
				p_app_context->settings.save_user_settings(*p_services);
				break;
			}
			case static_cast<int>(SettingsToggles::military_time): svc.world_clock.toggle_military_time(); break;
			case static_cast<int>(SettingsToggles::language): {
				m_menu.emplace(svc, p_app_context->localization.get_copy_of_available_languages(), options.at(static_cast<int>(SettingsToggles::language)).label.getGlobalBounds().position, p_app_context->settings.get_theme());
				break;
			}
			}
			options.at(static_cast<int>(SettingsToggles::autosprint))
				.label.setString(toggleables.autosprint.getString() + (svc.input_system.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.input_system.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			options.at(static_cast<int>(SettingsToggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
			if (m_lang) { options.at(static_cast<int>(SettingsToggles::language)).label.setString(toggleables.language.getString() + *m_lang); }
		} else if (was_selected(svc.input_system) && adjust_mode()) {
			m_mode = SettingsMenuMode::ready;
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		}
		if (svc.input_system.is_mouse_active()) {
			if (adjust_mode() && svc.input_system.get_mouse().left_button.released) { m_mode = SettingsMenuMode::ready; }
		}
		volume_settings.master.slider.handle_input(svc.input_system, svc.soundboard);
		volume_settings.music.slider.handle_input(svc.input_system, svc.soundboard);
		volume_settings.ambience.slider.handle_input(svc.input_system, svc.soundboard);
		volume_settings.sfx.slider.handle_input(svc.input_system, svc.soundboard);
	}

	MenuState::tick_update(svc, engine);
	if (m_menu) { m_menu->update(svc, options.at(static_cast<int>(SettingsToggles::language)).label.getGlobalBounds().position); }
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setLetterSpacing(1.2f);
	}
	auto final_delta = 0.0;
	if (adjust_mode()) {
		auto const update_volume = svc.ticker.every_x_ticks(16) && !m_adjustment.running();
		auto const delta = 0.010;
		auto const left_pressed = svc.input_system.menu_move(input::MoveDirection::left, input::DigitalActionQueryType::triggered);
		auto const right_pressed = svc.input_system.menu_move(input::MoveDirection::right, input::DigitalActionQueryType::triggered);
		if (left_pressed || right_pressed) { m_adjustment.start(); }
		auto const left = svc.input_system.menu_move(input::MoveDirection::left, input::DigitalActionQueryType::held);
		auto const right = svc.input_system.menu_move(input::MoveDirection::right, input::DigitalActionQueryType::held);
		auto const adjust_left = left_pressed || (left && update_volume);
		auto const adjust_right = right_pressed || (right && update_volume);
		final_delta = adjust_left ? -delta : adjust_right ? delta : 0.0;
	}

	// volume sliders
	volume_settings.master.slider.update(svc.mixer.volume.get_base());
	volume_settings.music.slider.update(svc.music_player.volume.get_base());
	volume_settings.ambience.slider.update(svc.ambience_player.volume.get_base());
	volume_settings.sfx.slider.update(svc.soundboard.volume.get_base());

	if (volume_settings.master.slider.is_engaged()) {
		current_selection.set(static_cast<int>(SettingsToggles::master));
		set_hovered(true);
		m_mode = SettingsMenuMode::adjust;
	}
	if (volume_settings.music.slider.is_engaged()) {
		current_selection.set(static_cast<int>(SettingsToggles::music));
		set_hovered(true);
		m_mode = SettingsMenuMode::adjust;
	}
	if (volume_settings.ambience.slider.is_engaged()) {
		current_selection.set(static_cast<int>(SettingsToggles::ambience));
		set_hovered(true);
		m_mode = SettingsMenuMode::adjust;
	}
	if (volume_settings.sfx.slider.is_engaged()) {
		current_selection.set(static_cast<int>(SettingsToggles::sfx));
		set_hovered(true);
		m_mode = SettingsMenuMode::adjust;
	}
	if (is(SettingsToggles::master)) {
		adjust_volume(svc.mixer.volume, final_delta, svc.soundboard);
		svc.music_player.adjust_volume(0.0);
		svc.ambience_player.volume.adjust_base(0.0);
	}
	if (is(SettingsToggles::music)) {
		adjust_volume(svc.music_player.volume, final_delta, svc.soundboard);
		svc.music_player.adjust_volume(0.0);
	}
	if (is(SettingsToggles::ambience)) { adjust_volume(svc.ambience_player.volume, final_delta, svc.soundboard); }
	if (is(SettingsToggles::sfx)) { adjust_volume(svc.soundboard.volume, final_delta, svc.soundboard); }

	if (auto const o = volume_settings.master.slider.get_output()) {
		if (static_cast<int>(*o * 100.f) != svc.mixer.volume.as_i_percent()) { svc.soundboard.play_sound("menu_tick"); }
		svc.mixer.volume.set_base(*o);
	}
	if (auto const o = volume_settings.music.slider.get_output()) {
		if (static_cast<int>(*o * 100.f) != svc.music_player.volume.as_i_percent()) { svc.soundboard.play_sound("menu_tick"); }
		svc.music_player.volume.set_base(*o);
	}
	if (auto const o = volume_settings.ambience.slider.get_output()) {
		if (static_cast<int>(*o * 100.f) != svc.ambience_player.volume.as_i_percent()) { svc.soundboard.play_sound("menu_tick"); }
		svc.ambience_player.volume.set_base(*o);
	}
	if (auto const o = volume_settings.sfx.slider.get_output()) {
		if (static_cast<int>(*o * 100.f) != svc.soundboard.volume.as_i_percent()) { svc.soundboard.play_sound("menu_tick"); }
		svc.soundboard.volume.set_base(*o);
	}
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	auto index = static_cast<int>(SettingsToggles::sfx);
	if (is(SettingsToggles::master)) {
		index = static_cast<int>(SettingsToggles::master);
	} else if (is(SettingsToggles::music)) {
		index = static_cast<int>(SettingsToggles::music);
	} else if (is(SettingsToggles::ambience)) {
		index = static_cast<int>(SettingsToggles::ambience);
	}

	adjust_mode() ? options.at(index).label.setFillColor(p_app_context->settings.get_theme().activated_text_color) : options.at(index).label.setFillColor(options.at(index).label.getFillColor());
	MenuState::render(svc, win);
	if (is(SettingsToggles::master)) { options.at(index).label.setString(volume_settings.master.label.getString() + std::to_string(static_cast<int>(svc.mixer.volume.get_base() * 100.0)) + "%"); }
	if (is(SettingsToggles::music)) { options.at(index).label.setString(volume_settings.music.label.getString() + std::to_string(static_cast<int>(svc.music_player.volume.get_base() * 100.0)) + "%"); }
	if (is(SettingsToggles::ambience)) { options.at(index).label.setString(volume_settings.ambience.label.getString() + std::to_string(static_cast<int>(svc.ambience_player.volume.get_base() * 100.0)) + "%"); }
	if (is(SettingsToggles::sfx)) { options.at(index).label.setString(volume_settings.sfx.label.getString() + std::to_string(static_cast<int>(svc.soundboard.volume.get_base() * 100.0)) + "%"); }

	volume_settings.master.slider.render(win);
	volume_settings.music.slider.render(win);
	volume_settings.ambience.slider.render(win);
	volume_settings.sfx.slider.render(win);

	if (p_context.console) {
		p_context.console.value()->render(win);
		p_context.console.value()->write(win, true);
	}
	if (m_menu) { m_menu->render(win); }
}

void SettingsMenu::adjust_volume(Volume& volume, double const amount, audio::Soundboard& soundboard) {
	auto const previous = volume.as_i_percent();
	volume.adjust_base(amount);
	if (volume.as_i_percent() != previous) { soundboard.play_sound("menu_tick"); }
}

} // namespace fornani::automa
