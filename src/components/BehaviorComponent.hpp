//
//  BehaviorComponent.hpp
//  components
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/StateMachine.hpp"
#include "../utils/AdjacencyList.hpp"
#include "../entities/behavior/EntityBehavior.hpp"
#include <vector>
#include <cmath>
#include <random>
#include <list>

namespace components {

class BehaviorComponent {
public:
    
    BehaviorComponent() = default;
    ~BehaviorComponent() {}

    void update() {};
    
};

class PlayerBehaviorComponent {
public:

    PlayerBehaviorComponent() { current_state = behavior::Behavior(); }

    void update() {
        if (current_state.params.complete && current_state.params.no_loop) {
            reset();
        }
    }

    void end_loop() {
        if (current_state.params.complete && current_state.params.no_loop) {
            reset();
        }
    }

    void reset() {
        if (ready()) {
            switch (facing) {
            case behavior::DIR::UP:
                current_state = behavior::Behavior(behavior::idle_up);
                break;
            case behavior::DIR::DOWN:
                current_state = behavior::Behavior(behavior::idle_up);
                break;
            default:
                current_state = behavior::Behavior(behavior::idle);
                break;
            }
        }
    }

    void run() {
        if (ready()) {
            switch (facing_lr) {
            case behavior::DIR_LR::LEFT:
                current_state = behavior::Behavior(behavior::running);
                break;
            case behavior::DIR_LR::RIGHT:
                current_state = behavior::Behavior(behavior::running);
                break;
            }
        }
    }

    void stop() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::stop);
        }
    }

    void turn() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::turning);
        }
    }

    void hurt() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::hurt);
        }
    }

    void jump() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::jumpsquat);
        }
    }

    void inspect() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::inspecting);
        }
    }

    void rise() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::rising);
        }
    }
    void suspend() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::suspended);
        }
    }
    void fall() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::falling);
        }
    }

    void land() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::landing);
        }
    }

    void air(float velocity) {
        if (ready()) {
            if (velocity < -suspension_threshold) {
                rise();
            }
            else if (velocity < suspension_threshold) {
                suspend();
            }
            else {
                fall();
            }
        }
    }

    void wall_slide() {
        if (ready()) {
            current_state = behavior::Behavior(behavior::wall_sliding);
        }
    }

    bool ready() {
        return !current_state.params.transitional || current_state.params.complete;
    }

    bool restricted() {
        return current_state.params.restrictive && !current_state.params.complete;
    }

    bool facing_left() {
        return facing_lr == behavior::DIR_LR::LEFT;
    }

    bool facing_right() {
        return facing_lr == behavior::DIR_LR::RIGHT;
    }

    bool facing_up() {
        return facing == behavior::DIR::UP_LEFT || facing == behavior::DIR::UP || facing == behavior::DIR::UP_RIGHT;
    }

    bool facing_down() {
        return facing == behavior::DIR::DOWN_LEFT || facing == behavior::DIR::DOWN || facing == behavior::DIR::DOWN_RIGHT;
    }

    bool facing_strictly_left() {
        return facing == behavior::DIR::LEFT;
    }

    bool facing_strictly_right() {
        return facing == behavior::DIR::RIGHT;
    }

    int get_frame() { return current_state.get_frame(); }

    const float suspension_threshold{ 1.0f };
    behavior::DIR facing{};
    behavior::DIR_LR facing_lr{};
    
    behavior::Behavior current_state;
    
};


class CritterBehaviorComponent {
public:
    CritterBehaviorComponent() { current_state = behavior::Behavior(behavior::frdog_idle); };
    CritterBehaviorComponent(int pid) : id(pid) {}
    
    void reset() {
        if(ready()) {
            switch (id) {
            case 0: current_state = behavior::Behavior(behavior::frdog_idle); break;
            case 1: current_state = behavior::Behavior(behavior::hulmet_idle); break;
            }
        }
    }

    void update() {
        if(current_state.params.complete && current_state.params.no_loop) {
            reset();
        }
    }

    void idle() {
        if(ready() && complete()) {
            switch (id) {
            case 0: current_state = behavior::Behavior(behavior::frdog_idle); break;
            case 1: current_state = behavior::Behavior(behavior::hulmet_idle); break;
            }
        }
    }

    void awaken() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_awakened);
        }
    }

    void bark() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_bark);
        }
    }

    void charge() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_charging);
        }
    }

    void bite() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_bite);
        }
    }

    void hurt() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_hurt);
        }
    }

    void turn() {
        if(ready()) {
            switch (id) {
            case 0: current_state = behavior::Behavior(behavior::frdog_turn); break;
            case 1: current_state = behavior::Behavior(behavior::hulmet_turn); break;
            }
        }
    }

    void run() {
        if(ready()) {
            switch (id) {
            case 0: current_state = behavior::Behavior(behavior::frdog_run); break;
            case 1: current_state = behavior::Behavior(behavior::hulmet_run); break;
            }
        }
    }

    void sleep() {
        if(ready()) {
            current_state = behavior::Behavior(behavior::frdog_asleep);
        }
    }
    
    bool ready() {
        return !current_state.params.transitional || current_state.params.complete;
    }
    
    bool restricted() {
        return current_state.params.restrictive && !current_state.params.complete;
    }

    bool complete() {
        return current_state.params.complete;
    }
    
    bool facing_left() {
        return facing_lr == behavior::DIR_LR::LEFT;
    }
    
    bool facing_right() {
        return facing_lr == behavior::DIR_LR::RIGHT;
    }
    
    int get_frame() { return current_state.get_frame(); }
    
    behavior::DIR_LR facing_lr{};
    
    behavior::Behavior current_state;

    int id{};
    
};

} // end components

/* BehaviorComponent_hpp */
