
#include <fornani/automa/GameplayState.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

GameplayState::GameplayState(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number), p_services{&svc} {
	svc.input_system.set_action_set(input::ActionSet::Platformer);
	svc.events.play_song_event.attach_to(p_slot, &GameplayState::play_song_by_id, this);
}

void GameplayState::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	// gamepad disconnected
	if (svc.input_system.process_gamepad_disconnection()) { pause(svc); }
	if (svc.input_system.digital(input::DigitalAction::pause).triggered) { pause(svc); }

	if (!m_map) {
		set_flag(GameplayStateFlags::early_tick_return);
		return;
	}

	svc.ambience_player.set_balance(m_map->get_ambience_balance());
	svc.music_player.set_balance(1.f - m_map->get_music_balance());

	if (p_pause_window) {
		m_map->set_target_balance(0.f, audio::BalanceTarget::music);
		m_map->set_target_balance(0.f, audio::BalanceTarget::ambience);
		m_map->update_balance(svc);
		p_pause_window.value()->update(svc, m_console);
		if (p_pause_window.value()->settings_requested()) {
			flags.set(GameStateFlags::settings_request);
			p_pause_window.value()->reset();
		}
		if (p_pause_window.value()->controls_requested()) {
			flags.set(GameStateFlags::controls_request);
			p_pause_window.value()->reset();
		}
		if (p_pause_window.value()->exit_requested()) {
			p_pause_window.reset();
			auto to_set = p_inventory_window || p_vendor_dialog ? input::ActionSet::Menu : input::ActionSet::Platformer;
			svc.input_system.set_action_set(to_set);
		}
		set_flag(GameplayStateFlags::early_tick_return);
	}
	GameState::tick_update(svc, engine);
}

void GameplayState::render(ServiceProvider& svc, sf::RenderWindow& win) {

	m_console || svc.state_flags.test(automa::StateFlags::cutscene) ? svc.state_flags.set(automa::StateFlags::hide_hud) : svc.state_flags.reset(automa::StateFlags::hide_hud);

	if (!svc.greyblock_mode() && !svc.hide_hud()) { hud.render(svc, *player, win); }

	if (p_vendor_dialog && p_gui_shader) { p_vendor_dialog.value()->render(svc, win, *player, *m_map, *p_gui_shader); }
	if (p_inventory_window && p_gui_shader) { p_inventory_window.value()->render(svc, win, *player, *p_gui_shader); }

	m_map->transition.render(win);
	if (p_pause_window) { p_pause_window.value()->render(svc, win); }
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win);
	}
	if (svc.debug_mode()) { /*m_map->debug();*/
	}

	svc.notifications.render(win);
}

void GameplayState::pause(ServiceProvider& svc) { p_pause_window = std::make_unique<gui::PauseWindow>(svc); }

void GameplayState::play_song_by_id(int id) { p_services->music_player.play_song_by_id(p_services->finder, id); }

} // namespace fornani::automa
