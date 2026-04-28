
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
		player->health.refill();
		svc.state_controller.actions.set(Actions::restart);
		m_reset.start();
		player->get_collider().physics.zero();
		player->controller.prevent_movement();
		player->map_reset();
		player->accumulated_forces.clear();
		player->set_direction({LR::right});
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

void Trial::pause(ServiceProvider& svc) {
	p_pause_window =
		std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{svc.data.gui_text["pause_menu"]["resume"].as_string(), svc.data.gui_text["pause_menu"]["settings"].as_string(),
																		 svc.data.gui_text["pause_menu"]["controls"].as_string(), svc.data.gui_text["pause_menu"]["quit"].as_string(), svc.data.gui_text["pause_menu"]["restart"].as_string()});
	svc.world_timer.pause();
}

} // namespace fornani::automa
