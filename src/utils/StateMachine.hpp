//
//  StateMachine.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>

namespace {

template<typename State>
class StateMachine {
    
public:
    
    StateMachine() { current_state = std::make_unique<State>(); }
    ~StateMachine() { current_state.reset(); }
    StateMachine& operator=(StateMachine&&) = delete;
    
    State& initialize();
    State& get_current_state() {
        assert(current_state);
        return *current_state;
        
    }
    State& set_current_state(std::unique_ptr<State> state) {
        current_state = std::move(state);
        return get_current_state();
    }
    
    std::unique_ptr<State> current_state{};
    
}; // End StateMachine

}

 /* StateMachine_hpp */
