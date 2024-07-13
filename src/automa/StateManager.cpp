
#include "StateManager.hpp"
#include "../setup/Game.hpp"

namespace automa {
StateManager::StateManager() {
	g_current_state = std::make_unique<MainMenu>();
}

StateManager::~StateManager() {}

void StateManager::process_state(ServiceProvider& svc, player::Player& player, fornani::Game& game) {
	if (svc.state_controller.actions.test(Actions::trigger_submenu)) {
		switch (svc.state_controller.submenu) {
		case menu_type::file_select:
			set_current_state(std::make_unique<FileMenu>(svc, player, "file"));
			game.playtest_sync();
			break;
		case menu_type::options: set_current_state(std::make_unique<OptionsMenu>(svc, player, "options")); break;
		case menu_type::settings: set_current_state(std::make_unique<SettingsMenu>(svc, player, "settings")); break;
		case menu_type::controls: set_current_state(std::make_unique<ControlsMenu>(svc, player, "controls")); break;
		case menu_type::credits: set_current_state(std::make_unique<CreditsMenu>(svc, player, "credits")); break;
		}
		svc.state_controller.actions.reset(Actions::trigger_submenu);
	}
	if (svc.state_controller.actions.test(Actions::exit_submenu)) {
		switch (svc.state_controller.submenu) {
		case menu_type::options: set_current_state(std::make_unique<OptionsMenu>(svc, player, "options")); break;
		default: set_current_state(std::make_unique<MainMenu>(svc, player, "main")); break;
		}
		svc.state_controller.actions.reset(Actions::exit_submenu);
	}
	if (svc.state_controller.actions.test(Actions::player_death)) {
		if (svc.demo_mode()) {
			svc.state_controller.next_state = svc.state_controller.demo_level;
		} else {
			if (svc.state_controller.actions.test(Actions::retry)) {
				svc.state_controller.next_state = svc.state_controller.save_point_id;
				svc.data.load_progress(player, svc.data.current_save);
				svc.state_controller.actions.reset(Actions::retry);
				player.animation.state = player::AnimState::idle;
				player.animation.triggers.reset(player::AnimTriggers::end_death);
				svc.stats.player.death_count.update();
			} else {
				return_to_main_menu(svc, player);
				return;
			}
			svc.music.stop();
		}
	}
	if (svc.state_controller.actions.consume(Actions::trigger)) {
		if (svc.state_controller.actions.test(Actions::main_menu)) {
			return_to_main_menu(svc, player);
			svc.state_controller.actions.reset(Actions::main_menu);
			return;
		}
		if (svc.data.get_file().is_new() && !svc.state_controller.actions.test(Actions::intro_done)) {
			set_current_state(std::make_unique<Intro>(svc, player, "intro"));
		} else {
			game.flags.set(fornani::GameFlags::in_game);
			set_current_state(std::make_unique<Dojo>(svc, player, "dojo"));
			get_current_state().init(svc, svc.state_controller.next_state);
			if (svc.demo_mode()) { player.set_position(svc.state_controller.player_position); }
			game.playtest_sync();
		}
	}
}

void StateManager::return_to_main_menu(ServiceProvider& svc, player::Player& player) {
	set_current_state(std::make_unique<MainMenu>(svc, player, "main"));
	svc.state_controller.actions.reset(Actions::player_death);
	svc.state_controller.actions.reset(Actions::trigger);
	svc.state_controller.actions.reset(Actions::retry);
	player.start_over();
	player.animation.state = player::AnimState::run;
}

auto StateManager::get_current_state() const -> GameState& {
	assert(g_current_state);
	return *g_current_state;
}

auto StateManager::set_current_state(std::unique_ptr<GameState> gameState) -> GameState& {
	g_current_state = std::move(gameState);
	return get_current_state();
}

}
