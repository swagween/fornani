#pragma once

#include "fornani/automa/states/MainMenu.hpp"
#include "fornani/automa/states/FileMenu.hpp"
#include "fornani/automa/states/OptionsMenu.hpp"
#include "fornani/automa/states/ControlsMenu.hpp"
#include "fornani/automa/states/CreditsMenu.hpp"
#include "fornani/automa/states/SettingsMenu.hpp"
#include "fornani/automa/states/StatSheet.hpp"
#include "fornani/automa/states/Intro.hpp"
#include "fornani/automa/states/Dojo.hpp"

namespace fornani {
class Game;
}

namespace fornani::player {
class Player;
}

namespace fornani::automa {

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

} // namespace fornani::automa
