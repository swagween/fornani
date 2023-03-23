//
//  StateMachine.cpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#include "StateMachine.hpp"

namespace {
//a unique pointer to a gamestate
std::unique_ptr<State<T> > current_state{};
}

namespace automa {
StateManager::StateManager() {
    //constructs a GameState object with the default contstructor, and wraps it in g_current_state
    current_state = std::make_unique<State>();
}

StateManager::~StateManager() {
    current_state.reset();
}

auto StateManager::get_current_state() -> automa::State& {
    //make sure the pointer is initialized, then return a pointer to it
    assert(current_state);
    return *current_state;
}

auto StateManager::set_current_state(std::unique_ptr<State> state) -> automa::State& {
    //make it so that instead of gameState pointing to the current state, now g_current_state points to the current state
    current_state = std::move(state);
    return get_current_state();
}
}

}//end automa
