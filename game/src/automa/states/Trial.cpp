
#include <fornani/automa/states/Trial.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>

namespace fornani::automa {

Trial::Trial(ServiceProvider& svc, player::Player& player, int room_number) : GameplayState(svc, player, room_number), m_reset{64} {

	m_map.emplace(svc, player);

	m_type = StateType::game;

	svc.data.load_trial_save(player);

	player.reset_flags();
	player.set_flag(player::PlayerFlags::trial);
	svc.soundboard.turn_on();
	if (!svc.data.exists(room_number)) {
		svc.data.rooms.push_back(room_number);
		svc.data.load_data();
	} else {
		m_map->load(svc, p_context, room_number);
	}

	svc.state_controller.player_position = m_map->get_player_start();

	player.set_camera_bounds(m_map->real_dimensions);
	player.force_camera_center();

	player.get_collider().physics.zero();
	player.set_position(m_map->get_player_start());
	player.reset_flags();
	player.set_direction(Direction{{1, 0}});

	// save was loaded from a json, or player died, so we successfully skipped door search
	if (!player.is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	svc.world_timer.restart();
}

void Trial::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	set_flag(GameplayStateFlags::early_tick_return, false);
	GameplayState::tick_update(svc, engine);
	if (has_flag_set(GameplayStateFlags::early_tick_return)) { return; }

	m_reset.update();
	svc.world_clock.update(svc);

	if (!m_map) { return; }

	if (!m_reset.running()) {
		player->update(*m_map);
		player->start_tick();
	}
	m_map->update(svc, p_context);

	m_map->debug_mode = debug_mode;

	player->end_tick();
	if (!p_context.console) { player->set_busy(false); }

	if (player->is_dead()) {
		p_context.transition.start();
		player->start_over();
		svc.state_controller.actions.set(Actions::restart);
	}

	m_map->background->update(svc);
}

void Trial::frame_update(ServiceProvider& svc) {}

void Trial::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!m_map) { return; }
	m_map->render_background(svc, win, p_world_shader, player->get_camera_position());
	m_map->render(svc, win, p_world_shader, player->get_camera_position());
	GameplayState::render(svc, win);
}

void Trial::reload(ServiceProvider& svc, int target_state) {
	m_map->clear();
	set_flag(GameplayStateFlags::transitioned_in, false);
	p_context.transition.hang();
	player->reset_flags();

	if (p_context.console) { p_context.console.reset(); }
	m_map->load(svc, p_context, target_state);

	hud.reset_position();
	svc.soundboard.turn_on();
	player->set_camera_bounds(m_map->real_dimensions);
	player->force_camera_center();
	player->get_collider().physics.zero();
	if (!svc.state_controller.actions.test(Actions::custom_player_position)) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		svc.demo_mode() ? player->place_at_demo_position() : player->set_position(player_pos);
	} else if (svc.state_controller.actions.test(Actions::custom_player_position)) {
		player->set_position(svc.state_controller.player_position);
		svc.state_controller.actions.reset(automa::Actions::custom_player_position);
	}

	// save was loaded from a json, or player died, so we successfully skipped door search
	svc.state_controller.actions.reset(Actions::save_loaded);
	if (!player->is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }
	player->visit_history.push_room(target_state);

	player->controller.prevent_movement();
	p_world_shader->set_darken(m_map->darken_factor);
	p_world_shader->set_texture_size(m_map->real_dimensions / constants::f_scale_factor);
	p_gui_shader->set_texture_size(svc.window->f_screen_dimensions() * 3.f); // 3 is the number of screen-sized "cells" in the inventory window
	svc.app_flags.reset(automa::AppFlags::custom_map_start);
}

void Trial::pause(ServiceProvider& svc) {
	p_pause_window =
		std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{svc.data.gui_text["pause_menu"]["resume"].as_string(), svc.data.gui_text["pause_menu"]["settings"].as_string(),
																		 svc.data.gui_text["pause_menu"]["controls"].as_string(), svc.data.gui_text["pause_menu"]["quit"].as_string(), svc.data.gui_text["pause_menu"]["restart"].as_string()});
	svc.world_timer.pause();
}

} // namespace fornani::automa
