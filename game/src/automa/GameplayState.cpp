
#include <fornani/automa/GameplayState.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::automa {

GameplayState::GameplayState(ServiceProvider& svc, player::Player& player, int room_number) : GameState(svc, player), p_services{&svc} {
	svc.input_system.set_action_set(input::ActionSet::Platformer);
	svc.events.play_song_event.attach_to(p_slot, &GameplayState::play_song_by_id, this);
	svc.events.pause_event.attach_to(p_slot, &GameplayState::pause, this);
	p_context.transition.set(graphics::TransitionState::black);
	svc.app_flags.set(AppFlags::in_game);

	// create shaders
	p_world_shader = LightShader(svc.finder);
	p_entity_shader = LightShader(svc.finder);
	p_gui_shader = LightShader(svc.finder);
}

void GameplayState::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	if (p_context.transition.has_waited(64) && !has_flag_set(GameplayStateFlags::transitioned_in)) {
		p_context.transition.end();
		set_flag(GameplayStateFlags::transitioned_in);
	}
	p_context.transition.update(*player);

	// cutscenes
	p_context.cutscene_catalog.update(svc, p_context, *m_map, *player);

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
		p_pause_window.value()->update(svc, p_context.console);
		if (p_pause_window.value()->settings_requested()) {
			flags.set(GameStateFlags::settings_request);
			p_pause_window.value()->reset();
		}
		if (p_pause_window.value()->controls_requested()) {
			flags.set(GameStateFlags::controls_request);
			p_pause_window.value()->reset();
		}
		if (p_pause_window.value()->exit_requested()) { unpause(svc); }
		if (svc.input_system.digital(input::DigitalAction::menu_close).triggered) { unpause(svc); }
		set_flag(GameplayStateFlags::early_tick_return);
	} else {
		svc.input_system.cancel_mouse();
	}

	GameState::tick_update(svc, engine);
}

void GameplayState::render(ServiceProvider& svc, sf::RenderWindow& win) {

	p_renderer.end();

	for (auto const& cutscene : p_context.cutscene_catalog.cutscenes) { cutscene->render_on_top(win, {}); }

	p_context.console || svc.state_flags.test(automa::StateFlags::cutscene) ? svc.state_flags.set(automa::StateFlags::hide_hud) : svc.state_flags.reset(automa::StateFlags::hide_hud);

	if (!svc.hide_hud()) { hud.render(svc, *player, win); }

	if (p_dialog && p_gui_shader) {
		p_renderer.begin(win, {});
		p_dialog.value()->render(svc, win, *player, *m_map, *p_gui_shader, p_renderer);
		p_renderer.end();
	}
	if (p_inventory_window && p_gui_shader) { p_inventory_window.value()->render(svc, win, *player, *p_gui_shader); }

	p_context.transition.render(win, get_context().get_black(svc));

	if (p_reward_sequence) { p_reward_sequence.value()->render(win); }

	if (p_pause_window) { p_pause_window.value()->render(svc, win); }
	if (p_context.console) {
		p_context.console.value()->render(win);
		p_context.console.value()->write(win);
	}
	if (svc.debug_mode()) { /*m_map->debug();*/
	}

	svc.notifications.render(win);
}

void GameplayState::reload(ServiceProvider& svc, int target_state) {
	m_map->clear();
	m_map->load(svc, p_context, target_state);

	svc.soundboard.clear_sounds(audio::SoundBus::gameplay);
	// set_flag(GameplayStateFlags::transitioned_in, false);
	player->reset_flags();

	if (p_context.console) { p_context.console.reset(); }

	p_context.biome.emplace(m_map->get_biome_string());

	// toxic haze
	p_haze_shader.reset();
	if (m_map->is_toxic()) { p_haze_shader.emplace(svc.finder, sf::Vector2u{svc.window->get().getSize()}, 2.1f); }

	hud.reset_position(); // reset hud position to corner
	svc.soundboard.turn_on();

	hud.reset_position();
	svc.soundboard.turn_on();

	player->set_camera_bounds(m_map->real_dimensions);
	player->map_reset();

	svc.camera_controller.constrain();

	if (!player->is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player->visit_history.push_room(target_state);

	if (m_map->has_property(world::MapProperties::lighting)) { m_palette.emplace(m_map->get_biome_string(), svc.finder); }
	p_world_shader->set_darken(m_map->darken_factor);
	p_entity_shader->set_darken(m_map->darken_factor);
	p_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);
	p_gui_shader->set_texture_size(svc.window->f_screen_dimensions() * 3.f); // 3 is the number of screen-sized "cells" in the inventory window
	svc.app_flags.reset(automa::AppFlags::custom_map_start);
}

void GameplayState::pause(ServiceProvider& svc) { p_pause_window = std::make_unique<gui::PauseWindow>(svc); }

void GameplayState::unpause(ServiceProvider& svc) {
	p_pause_window.reset();
	auto to_set = p_inventory_window || p_dialog ? input::ActionSet::Menu : input::ActionSet::Platformer;
	svc.input_system.set_action_set(to_set);
}

void GameplayState::play_song_by_id(int id) { p_services->music_player.play_song_by_id(p_services->finder, id); }

} // namespace fornani::automa
