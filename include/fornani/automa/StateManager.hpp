#pragma once

#include "states/MainMenu.hpp"
#include "states/FileMenu.hpp"
#include "states/OptionsMenu.hpp"
#include "states/ControlsMenu.hpp"
#include "states/CreditsMenu.hpp"
#include "states/SettingsMenu.hpp"
#include "states/StatSheet.hpp"
#include "states/Intro.hpp"
#include "states/Dojo.hpp"

namespace fornani {
class Game;
}
namespace player {
class Player;
}

namespace automa {

struct ServiceProvider;

class StateManager {

  public:
	StateManager(ServiceProvider& svc, player::Player& player);
	StateManager& operator=(StateManager&&) = delete;
	void process_state(ServiceProvider& svc, player::Player& player, fornani::Game& game);
	void return_to_main_menu(ServiceProvider& svc, player::Player& player);
	void print_stats(ServiceProvider& svc, player::Player& player);

	GameState& get_current_state() const;
	GameState& set_current_state(std::unique_ptr<automa::GameState> gameState);

	std::unique_ptr<automa::GameState> g_current_state{};
};

} // namespace automa
