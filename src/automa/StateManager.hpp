
#pragma once

#include "states/MainMenu.hpp"
#include "states/FileMenu.hpp"
#include "states/OptionsMenu.hpp"
#include "states/Dojo.hpp"

namespace automa {

class StateManager {
    
public:
    
    StateManager();
    ~StateManager();
    StateManager& operator=(StateManager&&) = delete;
    
    GameState& get_current_state();
    GameState& set_current_state(std::unique_ptr<automa::GameState> gameState);

    std::string get_current_state_string();
    
};

}
