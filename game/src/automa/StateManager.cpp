
#include "fornani/automa/StateManager.hpp"

#include "fornani/automa/states/ControlsMenu.hpp"
#include "fornani/automa/states/CreditsMenu.hpp"
#include "fornani/automa/states/Dojo.hpp"
#include "fornani/automa/states/FileMenu.hpp"
#include "fornani/automa/states/Intro.hpp"
#include "fornani/automa/states/MainMenu.hpp"
#include "fornani/automa/states/OptionsMenu.hpp"
#include "fornani/automa/states/PlayMenu.hpp"
#include "fornani/automa/states/SettingsMenu.hpp"
#include "fornani/automa/states/StatSheet.hpp"
#include "fornani/automa/states/Trial.hpp"
#include "fornani/automa/states/TrialsMenu.hpp"
#include "fornani/core/Game.hpp"

namespace fornani::automa {

StateManager::StateManager(ServiceProvider& svc, player::Player& player, MenuType type) {
	switch (type) {
	case MenuType::main: g_current_state = std::make_unique<MainMenu>(svc, player); break;
	case MenuType::settings: g_current_state = std::make_unique<SettingsMenu>(svc, player); break;
	case MenuType::controls: g_current_state = std::make_unique<ControlsMenu>(svc, player); break;
	}
}

void StateManager::process_state(ServiceProvider& svc, player::Player& player, fornani::Game& game) {
	if (svc.state_controller.actions.test(Actions::trigger_submenu)) {
		switch (svc.state_controller.submenu) {
		case MenuType::play: set_current_state(std::make_unique<PlayMenu>(svc, player)); break;
		case MenuType::file_select: set_current_state(std::make_unique<FileMenu>(svc, player)); break;
		case MenuType::options: set_current_state(std::make_unique<OptionsMenu>(svc, player)); break;
		case MenuType::settings: set_current_state(std::make_unique<SettingsMenu>(svc, player)); break;
		case MenuType::controls: set_current_state(std::make_unique<ControlsMenu>(svc, player)); break;
		case MenuType::credits: set_current_state(std::make_unique<CreditsMenu>(svc, player)); break;
		case MenuType::trials: set_current_state(std::make_unique<TrialsMenu>(svc, player)); break;
		default: break;
		}
		svc.state_controller.actions.reset(Actions::trigger_submenu);
	}
	if (svc.state_controller.actions.test(Actions::exit_submenu)) {
		switch (svc.state_controller.submenu) {
		case MenuType::options: set_current_state(std::make_unique<OptionsMenu>(svc, player)); break;
		case MenuType::play: set_current_state(std::make_unique<PlayMenu>(svc, player)); break;
		default: set_current_state(std::make_unique<MainMenu>(svc, player)); break;
		}
		svc.state_controller.actions.reset(Actions::exit_submenu);
	}
	if (svc.state_controller.actions.test(Actions::player_death)) {
		if (svc.demo_mode()) {
			if (svc.state_controller.actions.test(Actions::retry)) {
				svc.state_controller.next_state = svc.state_controller.demo_level;
				svc.state_controller.actions.reset(Actions::retry);
				player.set_idle();
				player.set_animation_flag(player::AnimTriggers::end_death, false);
			} else {
				return_to_main_menu(svc, player);
				return;
			}
		} else {
			if (svc.state_controller.actions.test(Actions::retry)) {
				svc.state_controller.next_state = svc.state_controller.save_point_id;
				svc.data.load_progress(player, svc.data.current_save, false, false);
				svc.state_controller.actions.reset(Actions::retry);
				player.set_idle();
				player.set_animation_flag(player::AnimTriggers::end_death, false);
				svc.data.write_death_count(player);
			} else {
				return_to_main_menu(svc, player);
				svc.data.write_death_count(player);
				return;
			}
			svc.music_player.stop();
		}
	}
	if (svc.state_controller.actions.consume(Actions::trials)) { set_current_state(std::make_unique<Trial>(svc, player, "trial", svc.state_controller.next_state)); }
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
			set_current_state(std::make_unique<Intro>(svc, player, "intro", 138));
		} else {
			game.flags.set(fornani::GameFlags::in_game);
			set_current_state(std::make_unique<Dojo>(svc, player, "dojo", svc.state_controller.next_state));
		}
	}
}

void StateManager::return_to_main_menu(ServiceProvider& svc, player::Player& player) {
	player.start_over();
	player.request_animation(player::AnimState::run);
	player.unregister_with_map();
	if (svc.demo_mode()) {
		svc.state_controller.actions.set(Actions::shutdown);
	} else {
		set_current_state(std::make_unique<MainMenu>(svc, player));
	}
	svc.state_controller.actions = {};
	svc.state_flags = {};
}

void StateManager::print_stats(ServiceProvider& svc, player::Player& player) {
	set_current_state(std::make_unique<StatSheet>(svc, player));
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

} // namespace fornani::automa
