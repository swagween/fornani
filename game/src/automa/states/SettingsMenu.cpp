
#include "fornani/automa/states/SettingsMenu.hpp"
#include <fornani/setup/AppContext.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::automa {

SettingsMenu::SettingsMenu(ServiceProvider& svc, player::Player& player, AppContext& ctx)
	: MenuState(svc, player, ctx, "settings"), toggleables{.autosprint = options.at(static_cast<int>(SettingsToggles::autosprint)).label,
														   .tutorial = options.at(static_cast<int>(SettingsToggles::tutorial)).label,
														   .gamepad = options.at(static_cast<int>(SettingsToggles::gamepad)).label,
														   .fullscreen = options.at(static_cast<int>(SettingsToggles::fullscreen)).label,
														   .military_time = options.at(static_cast<int>(SettingsToggles::military_time)).label,
														   .language = options.at(static_cast<int>(SettingsToggles::language)).label},
	  music_label{options.at(static_cast<int>(SettingsToggles::music)).label}, ambience_label{options.at(static_cast<int>(SettingsToggles::ambience)).label}, sfx_label{options.at(static_cast<int>(SettingsToggles::sfx)).label},
	  toggle_options{.enabled{svc.text.fonts.title.font}, .disabled{svc.text.fonts.title.font}}, sliders{.music_volume{svc.text.fonts.title.font}, .ambience_volume{svc.text.fonts.title.font}, .sfx_volume{svc.text.fonts.title.font}},
	  m_adjustment{80} {
	m_parent_menu = MenuType::options;
	toggle_options.enabled.setString(svc.data.gui_text["settings"]["enabled"].as_string());
	toggle_options.disabled.setString(svc.data.gui_text["settings"]["disabled"].as_string());
	m_lang = ctx.localization.get_language_title();

	options.at(static_cast<int>(SettingsToggles::autosprint)).label.setString(toggleables.autosprint.getString() + (svc.input_system.is_autosprint_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::tutorial)).label.setString(toggleables.tutorial.getString() + (svc.tutorial() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::gamepad)).label.setString(toggleables.gamepad.getString() + (svc.input_system.is_gamepad_input_enabled() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::music)).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::ambience)).label.setString(ambience_label.getString() + std::to_string(static_cast<int>(svc.ambience_player.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::sfx)).label.setString(sfx_label.getString() + std::to_string(static_cast<int>(svc.soundboard.volume.get_base() * 100.0)) + "%");
	options.at(static_cast<int>(SettingsToggles::fullscreen)).label.setString(toggleables.fullscreen.getString() + (svc.fullscreen() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
	options.at(static_cast<int>(SettingsToggles::military_time)).label.setString(toggleables.military_time.getString() + (svc.world_clock.is_military() ? toggle_options.enabled.getString() : toggle_options.disabled.getString()));
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
		if (m_menu) { m_menu->handle_inputs(svc.input_system, svc.soundboard); }
		if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
			if (m_menu) { m_menu.reset(); }
		}
		if (was_selected(svc.input_system, true)) {
			if (m_menu) {
				auto selected_lang = 0;
				for (auto const& lang : p_app_context->localization.get_available_languages()) {
					auto tag = p_app_context->localization.get_tag_from_index(lang.index);
					if (lang.index == m_menu->get_selection()) { svc.events.set_langauge_event.dispatch(*p_app_context, tag); }
				}
				m_menu.reset();
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
	}
	MenuState::tick_update(svc, engine);
	if (m_menu) { m_menu->update(svc, options.at(static_cast<int>(SettingsToggles::language)).label.getGlobalBounds().position); }
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setLetterSpacing(1.2f);
	}
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
		auto const final_delta = adjust_left ? -delta : adjust_right ? delta : 0.0;
		auto const adjusted = (adjust_left || adjust_right);
		if (is(SettingsToggles::music)) {
			if (adjusted && !svc.music_player.volume.at_range()) { svc.soundboard.play_sound("menu_tick"); }
			svc.music_player.adjust_volume(final_delta);
		}
		if (is(SettingsToggles::ambience)) {
			if (adjusted && !svc.ambience_player.volume.at_range()) { svc.soundboard.play_sound("menu_tick"); }
			svc.ambience_player.volume.adjust_base(final_delta);
		}
		if (is(SettingsToggles::sfx)) {
			if (adjusted && !svc.soundboard.volume.at_range()) { svc.soundboard.play_sound("menu_tick"); }
			svc.soundboard.volume.adjust_base(final_delta);
		}
	}
}

void SettingsMenu::frame_update(ServiceProvider& svc) {}

void SettingsMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	auto index = is(SettingsToggles::music) ? static_cast<int>(SettingsToggles::music) : is(SettingsToggles::ambience) ? static_cast<int>(SettingsToggles::ambience) : static_cast<int>(SettingsToggles::sfx);

	adjust_mode() ? options.at(index).label.setFillColor(p_app_context->settings.get_theme().activated_text_color) : options.at(index).label.setFillColor(options.at(index).label.getFillColor());
	MenuState::render(svc, win);
	if (is(SettingsToggles::music)) { options.at(index).label.setString(music_label.getString() + std::to_string(static_cast<int>(svc.music_player.volume.get_base() * 100.0)) + "%"); }
	if (is(SettingsToggles::ambience)) { options.at(index).label.setString(ambience_label.getString() + std::to_string(static_cast<int>(svc.ambience_player.volume.get_base() * 100.0)) + "%"); }
	if (is(SettingsToggles::sfx)) { options.at(index).label.setString(sfx_label.getString() + std::to_string(static_cast<int>(svc.soundboard.volume.get_base() * 100.0)) + "%"); }

	if (p_context.console) {
		p_context.console.value()->render(win);
		p_context.console.value()->write(win, true);
	}
	if (m_menu) { m_menu->render(win); }
}

} // namespace fornani::automa
