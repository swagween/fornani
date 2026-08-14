
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

	reload(svc, room_number);

	svc.state_controller.player_position = m_map->get_player_start();

	player.get_collider().physics.zero();
	player.set_position(m_map->get_player_start());
	player.reset_flags();
	player.set_direction(Direction{{1, 0}});
	svc.world_timer.restart();
}

void Trial::tick_update(ServiceProvider& svc, capo::IEngine& engine) {

	set_flag(GameplayStateFlags::early_tick_return, false);
	GameplayState::tick_update(svc, engine);
	if (has_flag_set(GameplayStateFlags::early_tick_return)) { return; }

	m_reset.update();

	if (!m_map) { return; }

	if (!p_pause_window) { svc.world_timer.resume(); }

	if (!m_reset.running()) {
		player->update(*m_map);
		player->start_tick();
	}
	m_map->update(svc, p_context);

	m_map->debug_mode = debug_mode;

	player->end_tick();
	if (!p_context.console) { player->set_busy(false); }

	if (player->is_dead() && !m_flags.test(TrialFlags::player_death)) {
		p_context.transition.start();
		m_flags.set(TrialFlags::player_death);
	}
	if (p_context.transition.is_black() && m_flags.test(TrialFlags::player_death)) {
		player->start_over();
		svc.state_controller.actions.set(Actions::restart);
		m_flags.reset(TrialFlags::player_death);
		set_flag(GameplayStateFlags::transitioned_in);
	}

	m_map->background->update(svc);
}

void Trial::frame_update(ServiceProvider& svc) {}

void Trial::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!m_map) { return; }
	p_renderer.begin(win, player->get_camera_position());
	m_map->render_background(p_renderer, svc, win, p_world_shader, player->get_camera_position());
	m_map->render(p_renderer, svc, win, p_world_shader, player->get_camera_position());
	GameplayState::render(svc, win);
}

void Trial::reload(ServiceProvider& svc, int target_state) {
	GameplayState::reload(svc, target_state);
	if (!svc.state_controller.actions.test(Actions::custom_player_position)) {
		float ppx = svc.data.get_save()["player_data"]["position"]["x"].as<float>();
		float ppy = svc.data.get_save()["player_data"]["position"]["y"].as<float>();
		sf::Vector2f player_pos = {ppx, ppy};
		svc.demo_mode() ? player->place_at_demo_position() : player->set_position(player_pos);
		NANI_LOG_INFO(m_logger, "Trial reset top");
	} else {
		player->set_position(svc.state_controller.player_position);
		svc.state_controller.actions.reset(automa::Actions::custom_player_position);
		NANI_LOG_INFO(m_logger, "Trial reset bottom");
	}
	svc.camera_controller.set_position(player->get_camera_focus_point());
	player->controller.stall_input();
	svc.input_system.flush_inputs();
}

void Trial::pause(ServiceProvider& svc) {
	p_pause_window =
		std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{svc.data.gui_text["pause_menu"]["resume"].as_string(), svc.data.gui_text["pause_menu"]["settings"].as_string(),
																		 svc.data.gui_text["pause_menu"]["controls"].as_string(), svc.data.gui_text["pause_menu"]["quit"].as_string(), svc.data.gui_text["pause_menu"]["restart"].as_string()});
	svc.world_timer.pause();
}

} // namespace fornani::automa
