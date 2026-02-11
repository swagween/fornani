
#include <fornani/automa/StateManager.hpp>
#include <fornani/automa/states/ControlsMenu.hpp>
#include <fornani/automa/states/CreditsMenu.hpp>
#include <fornani/automa/states/Dojo.hpp>
#include <fornani/automa/states/FileMenu.hpp>
#include <fornani/automa/states/Intro.hpp>
#include <fornani/automa/states/MainMenu.hpp>
#include <fornani/automa/states/OptionsMenu.hpp>
#include <fornani/automa/states/PlayMenu.hpp>
#include <fornani/automa/states/SettingsMenu.hpp>
#include <fornani/automa/states/StatSheet.hpp>
#include <fornani/automa/states/ThemesMenu.hpp>
#include <fornani/automa/states/Trial.hpp>
#include <fornani/automa/states/TrialsMenu.hpp>
#include <fornani/core/Game.hpp>
#include <fornani/events/SystemEvent.hpp>

namespace fornani::automa {

StateManager::StateManager(ServiceProvider& svc, player::Player& player, MenuType type) : m_player{&player} {

	svc.events.return_to_main_menu_event.attach_to(m_slot, &StateManager::return_to_main_menu, this);
	svc.events.reload_save_event.attach_to(m_slot, &StateManager::reload_save, this);

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
		case MenuType::themes: set_current_state(std::make_unique<ThemesMenu>(svc, player)); break;
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
			if (m_flags.consume(StateManagerFlags::retry)) {
				svc.state_controller.next_state = svc.state_controller.demo_level;
				player.place_at_demo_position();
				player.set_idle();
				player.set_animation_flag(player::AnimTriggers::end_death, false);
				svc.state_controller.actions.reset(Actions::player_death);
			} else {
				return_to_main_menu();
				svc.state_controller.actions.reset(Actions::player_death);
				return;
			}
		} else {
			if (m_flags.consume(StateManagerFlags::retry)) {
				NANI_LOG_INFO(m_logger, "Reloading save...");
				player.start_over();
				svc.data.reload_progress(player);
				get_current_state().reload(svc, svc.state_controller.save_point_id);
				svc.data.write_death_count(player);
				svc.state_controller.actions.reset(Actions::player_death);
				svc.state_controller.actions.reset(Actions::trigger);
			} else {
				return_to_main_menu();
				svc.data.write_death_count(player);
				svc.state_controller.actions.reset(Actions::player_death);
				return;
			}
		}
	}
	if (svc.state_controller.actions.consume(Actions::trials)) { set_current_state(std::make_unique<Trial>(svc, player, "trial", svc.state_controller.next_state)); }
	if (svc.state_controller.actions.consume(Actions::trigger)) {
		if (svc.state_controller.actions.test(Actions::print_stats)) {
			print_stats(svc, player);
			return;
		}
		if (svc.data.get_file().is_new() && !svc.state_controller.actions.test(Actions::intro_done)) {
			set_current_state(std::make_unique<Intro>(svc, player, "intro", 138));
		} else {
			game.flags.set(fornani::GameFlags::in_game);
			if (get_current_state().is(StateType::dojo)) {
				get_current_state().reload(svc, svc.state_controller.next_state);
			} else {
				set_current_state(std::make_unique<Dojo>(svc, player, "dojo", svc.state_controller.next_state));
			}
		}
	}
	if (m_flags.consume(StateManagerFlags::return_to_main_menu)) {
		m_player->start_over();
		m_player->request_animation(player::AnimState::run);
		m_player->unregister_with_map();
		if (svc.demo_mode()) {
			svc.state_controller.actions.set(Actions::shutdown);
		} else {
			set_current_state(std::make_unique<MainMenu>(svc, *m_player));
		}
		svc.state_flags = {};
	}
}

void StateManager::return_to_main_menu() {
	NANI_LOG_INFO(m_logger, "Returned to Main Menu.");
	m_flags.set(StateManagerFlags::return_to_main_menu);
}

void StateManager::print_stats(ServiceProvider& svc, player::Player& player) {
	set_current_state(std::make_unique<StatSheet>(svc, player));
	svc.state_controller.actions.reset(Actions::print_stats);
	svc.state_controller.actions.reset(Actions::trigger);
}

void StateManager::reload_save(ServiceProvider& svc, int id) {
	NANI_LOG_INFO(m_logger, "Reloaded save file {}.", id);
	m_flags.set(StateManagerFlags::retry);
}

auto StateManager::get_current_state() const -> GameState& {
	assert(g_current_state);
	return *g_current_state;
}

auto StateManager::set_current_state(std::unique_ptr<GameState> gameState) -> GameState& {
	g_current_state->on_exit();
	g_current_state = std::move(gameState);
	return get_current_state();
}

} // namespace fornani::automa
