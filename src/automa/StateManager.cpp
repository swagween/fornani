//
//  StateManager.cpp
//  automa
//
//  Created by Alex Frasca on 12/26/22.
//

#include "StateManager.hpp"

namespace {
//a unique pointer to a gamestate
std::unique_ptr<automa::GameState> g_current_state{};
}

namespace automa {
StateManager::StateManager() {
    //constructs a GameState object with the default contstructor, and wraps it in g_current_state
    g_current_state = std::make_unique<flstates::MainMenu>();
}

StateManager::~StateManager() {
    //g_current_state.reset();
}

auto StateManager::get_current_state() -> automa::GameState& {
    //make sure the pointer is initialized, then return a pointer to it
    assert(g_current_state);
    return *g_current_state;
}

auto StateManager::set_current_state(std::unique_ptr<automa::GameState> gameState) -> automa::GameState& {
    //make it so that instead of gameState pointing to the current state, now g_current_state points to the current state
    g_current_state = std::move(gameState);
    return get_current_state();
}

std::string StateManager::get_current_state_string() {
    switch( g_current_state.get()->state ) {
        case STATE::STATE_MENU:
            return "MENU";
            break;
        case STATE::STATE_DOJO:
            return "DOJO";
            break;
        default:
            return "NULL";
            break;
    }
    return "NULL";
}

}//end automa
