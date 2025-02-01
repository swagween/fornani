
#include "fornani/automa/StateManager.hpp"

#include <fornani/utils/Tracy.hpp>

#include "fornani/setup/Game.hpp"

namespace automa {

StateManager::StateManager(ServiceProvider& svc, player::Player& player) : g_current_state{std::make_unique<MainMenu>(svc, player, "main")} {}

void StateManager::process_state(ServiceProvider& svc, player::Player& player, fornani::Game& game) {
	NANI_ZoneScopedN("StateManager::process_state");
	if (svc.state_controller.actions.test(Actions::trigger_submenu)) {
		switch (svc.state_controller.submenu) {
		case menu_type::file_select:
			set_current_state(std::make_unique<FileMenu>(svc, player, "file"));
			game.playtest_sync();
			break;
		case menu_type::options: set_current_state(std::make_unique<OptionsMenu>(svc, player, "options")); break;
		case menu_type::settings: set_current_state(std::make_unique<SettingsMenu>(svc, player, "settings")); break;
		case menu_type::controls: set_current_state(std::make_unique<ControlsMenu>(svc, player, "controls_platformer")); break;
		case menu_type::credits: set_current_state(std::make_unique<CreditsMenu>(svc, player, "credits")); break;
		default: break;
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
			if (svc.state_controller.actions.test(Actions::retry)) {
				svc.state_controller.next_state = svc.state_controller.demo_level;
				svc.state_controller.actions.reset(Actions::retry);
				player.animation.state = player::AnimState::idle;
				player.animation.triggers.reset(player::AnimTriggers::end_death);
			} else {
				return_to_main_menu(svc, player);
				return;
			}
		} else {
			if (svc.state_controller.actions.test(Actions::retry)) {
				svc.state_controller.next_state = svc.state_controller.save_point_id;
				svc.data.load_progress(player, svc.data.current_save, false, false);
				svc.state_controller.actions.reset(Actions::retry);
				player.animation.state = player::AnimState::idle;
				player.animation.triggers.reset(player::AnimTriggers::end_death);
				svc.data.write_death_count(player);
			} else {
				return_to_main_menu(svc, player);
				svc.data.write_death_count(player);
				return;
			}
			svc.music.stop();
		}
	}
	if (svc.state_controller.actions.consume(Actions::trigger)) {
		if (svc.state_controller.actions.test(Actions::print_stats)) {
			print_stats(svc, player);
			return;
		}
		if (svc.state_controller.actions.test(Actions::main_menu)) {
			return_to_main_menu(svc, player);
			svc.state_controller.actions.reset(Actions::main_menu);
			return;
		}
		if (svc.data.get_file().is_new() && !svc.state_controller.actions.test(Actions::intro_done)) {
			set_current_state(std::make_unique<Intro>(svc, player, "intro"));
		} else {
			game.flags.set(fornani::GameFlags::in_game);
			set_current_state(std::make_unique<Dojo>(svc, player, "dojo", svc.state_controller.next_state));
			game.playtest_sync();
		}
	}
}

void StateManager::return_to_main_menu(ServiceProvider& svc, player::Player& player) {
	if (svc.demo_mode()) {
		svc.state_controller.actions.set(Actions::shutdown);
	} else {
		set_current_state(std::make_unique<MainMenu>(svc, player, "main"));
	}
	svc.state_controller.actions.reset(Actions::player_death);
	svc.state_controller.actions.reset(Actions::trigger);
	svc.state_controller.actions.reset(Actions::retry);
	player.start_over();
	player.animation.state = player::AnimState::run;
}

void StateManager::print_stats(ServiceProvider& svc, player::Player& player) {
	set_current_state(std::make_unique<StatSheet>(svc, player, "stat"));
	svc.state_controller.actions.reset(Actions::print_stats);
	svc.state_controller.actions.reset(Actions::trigger);
}

auto StateManager::get_current_state() const -> GameState& {
	assert(g_current_state);
	return *g_current_state;
}

auto StateManager::set_current_state(std::unique_ptr<GameState> gameState) -> GameState& {
	g_current_state = std::move(gameState);
	return get_current_state();
}

} // namespace automa
