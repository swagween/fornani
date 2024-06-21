#pragma once

#include "states/MainMenu.hpp"
#include "states/FileMenu.hpp"
#include "states/OptionsMenu.hpp"
#include "states/ControlsMenu.hpp"
#include "states/CreditsMenu.hpp"
#include "states/SettingsMenu.hpp"
#include "states/Dojo.hpp"

namespace automa {

class StateManager {

  public:
	StateManager();
	~StateManager();
	StateManager& operator=(StateManager&&) = delete;

	GameState& get_current_state();
	GameState& set_current_state(std::unique_ptr<automa::GameState> gameState);

	std::unique_ptr<automa::GameState> g_current_state{};
};

} // namespace automa
