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

enum class BEHAVIOR_TYPE {
    PLAYER_BEHAVIOR
};

class BehaviorComponent {
public:
    
    BehaviorComponent() {};
    BehaviorComponent(int num) : num_behaviors(num) {};
    
    virtual void instantiate_behaviors() {};
    
//    StateMachine<behavior::Behavior> machine{};
    
    int num_behaviors{};
    BEHAVIOR_TYPE type;
    
};

class PlayerBehaviorComponent : public BehaviorComponent {
public:
    
    PlayerBehaviorComponent() {};
    PlayerBehaviorComponent(const PlayerBehaviorComponent& PBC) {
        
    }
    
    void instantiate_behaviors() {
        
    }
    
    behavior::PlayerBehavior player_state{};
    std::unique_ptr<behavior::Behavior> current_state = std::make_unique<behavior::Behavior>();
    
};

} // end components

/* BehaviorComponent_hpp */
