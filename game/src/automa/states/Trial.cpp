
#include <fornani/automa/states/Trial.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/Event.hpp>
#include <fornani/utils/Random.hpp>

namespace fornani::automa {

Trial::Trial(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number, std::string_view room_name) : GameState(svc, player, scene, room_number), map(svc, player), m_services(&svc), m_reset{256} {

	m_type = StateType::game;

	svc.data.load_trial_save(player);

	player.reset_flags();
	svc.soundboard.turn_on();
	if (!svc.data.exists(room_number)) {
		svc.data.rooms.push_back(room_number);
		svc.data.load_data(room_name.data());
	} else {
		map.load(svc, room_number);
	}

	svc.state_controller.player_position = map.get_player_start();

	player.set_camera_bounds(map.real_dimensions);
	player.force_camera_center();

	player.collider.physics.zero();
	player.set_position(map.get_player_start());

	// save was loaded from a json, or player died, so we successfully skipped door search
	if (!player.is_dead()) { svc.state_controller.actions.reset(Actions::player_death); }

	player.controller.prevent_movement();
	svc.app_flags.reset(AppFlags::in_game);
	svc.world_timer.restart();
}

void Trial::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	GameState::tick_update(svc, engine);
	m_reset.update();
	// gamepad disconnected
	if (svc.controller_map.process_gamepad_disconnection()) { pause_window = std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{"resume", "settings", "controls", "quit", "restart"}); }

	svc.a11y.set_action_ctx_bar_enabled(false);

	svc.app_flags.set(AppFlags::in_game);

	// set action set
	if (pause_window || m_console) {
		svc.controller_map.set_action_set(config::ActionSet::Menu);
		svc.controller_map.set_joystick_throttle({});
	} else {
		svc.controller_map.set_action_set(config::ActionSet::Platformer);
	}

	if (pause_window) {
		if (m_console) { m_console.value()->update(svc); }
		pause_window.value()->update(svc, m_console);
		if (pause_window.value()->settings_requested()) {
			flags.set(GameStateFlags::settings_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->controls_requested()) {
			flags.set(GameStateFlags::controls_request);
			pause_window.value()->reset();
		}
		if (pause_window.value()->exit_requested()) { pause_window = {}; }
		return;
	}

	svc.world_clock.update(svc);

	// in-game menus
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_toggle_pause).triggered) {
		pause_window = std::make_unique<gui::PauseWindow>(svc, std::vector<std::string>{"resume", "settings", "controls", "quit", "restart"});
	}

	if (!m_console && svc.state_controller.actions.test(Actions::main_menu)) { svc.state_controller.actions.set(Actions::trigger); }

	if (!m_reset.running()) { player->update(map); }
	map.update(svc, m_console);

	player->controller.clean();
	player->flags.triggers = {};
	if (player->is_dead()) {
		map.transition.start();
		player->health.refill();
		svc.state_controller.actions.set(Actions::restart);
		m_reset.start();
		player->collider.physics.zero();
		player->controller.prevent_movement();
		player->map_reset();
		player->accumulated_forces.clear();
	}

	map.background->update(svc);
}

void Trial::frame_update(ServiceProvider& svc) {}

void Trial::render(ServiceProvider& svc, sf::RenderWindow& win) {
	map.render_background(svc, win, player->get_camera_position());
	map.render(svc, win, player->get_camera_position());

	map.soft_reset.render(win);
	if (pause_window) { pause_window.value()->render(svc, win); }
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win);
	}
	if (svc.debug_mode()) { map.debug(); }
}

} // namespace fornani::automa
