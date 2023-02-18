//
//  BehaviorComponent.hpp
//  components
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/StateMachine.hpp"
#include "../utils/AdjacencyList.hpp"
#include "../entities/behavior/PlayerBehavior.hpp"
#include <vector>
#include <cmath>
#include <random>
#include <list>

namespace components {

class BehaviorComponent {
public:
    
    BehaviorComponent() = default;
    ~BehaviorComponent() { current_state.reset(); }

    virtual void update() {};
    
    std::unique_ptr<behavior::Behavior> current_state = std::make_unique<behavior::Behavior>();
    
};

class PlayerBehaviorComponent : public BehaviorComponent {
public:
    
    void update(int frame) {
        assert(current_state);
        current_state.get()->update(frame);
    }
    
    behavior::PlayerBehavior player_state{};
    
};

} // end components

/* BehaviorComponent_hpp */
