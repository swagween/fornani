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
    
    void update() {
        if(current_state.get()->params.complete && current_state.get()->params.no_loop) {
            reset();
        }
    }
    
    void end_loop() {
        if(current_state.get()->params.complete && current_state.get()->params.no_loop) {
            reset();
        }
    }
    
    void reset() {
        if(ready()) {
            switch(facing) {
                case behavior::DIR::UP:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle_up));
                    break;
                case behavior::DIR::DOWN:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle_up));
                    break;
                default:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
                    break;
            }
            flip_left();
        }
    }
    
    void run() {
        if(ready()) {
            switch(facing_lr) {
                case behavior::DIR_LR::LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running));
                    break;
                case behavior::DIR_LR::RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running));
                    break;
            }
            flip_left();
        }
    }
    
    void stop() {
        if(ready()) {
            switch(facing) {
                case behavior::DIR::UP_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_up));
                    break;
                case behavior::DIR::DOWN_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_down));
                    break;
                case behavior::DIR::UP_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_up));
                    break;
                case behavior::DIR::DOWN_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_down));
                    break;
                default:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop));
                    break;
            }
        }
    }
    
    void turn() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::turning));
            flip_left();
        }
    }
    
    void jump() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::jumpsquat));
        }
    }
    void rise() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::rising));
        }
    }
    void suspend() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::suspended));
        }
    }
    void fall() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::falling));
        }
    }
    
    void land() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::landing));
        }
    }
    
    void air(float velocity) {
        if(ready()) {
            if(velocity < -suspension_threshold*2) {
                rise();
            } else if (velocity < suspension_threshold) {
                suspend();
            } else {
                fall();
            }
        }
    }
    
    void wall_slide() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::wall_sliding));
        }
    }
    
    void flip_left() {
//        if(facing_left()) {
//            int lookup = current_state->params.lookup_value;
//            if(lookup < 110) {
//                current_state->params.lookup_value = 210 + (lookup % 10) - (lookup - (lookup % 10));
//            }
//        } else {
//            int lookup = current_state->params.lookup_value;
//            if(lookup >= 110) {
//                current_state->params.lookup_value = 0 + (lookup % 10) + (210 - lookup + (lookup % 10));
//            }
//        }
    }
    
    bool ready() {
        return !current_state->params.transitional || current_state->params.complete;
    }
    
    bool restricted() {
        return current_state->params.restrictive && !current_state->params.complete;
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
    
    int get_frame() { return current_state->get_frame(); }
    
    const float suspension_threshold = 3.0f;
    behavior::DIR facing{};
    behavior::DIR_LR facing_lr{};
    
};


class CritterBehaviorComponent : public BehaviorComponent {
    
    void update() {}
    
    
    
};

} // end components

/* BehaviorComponent_hpp */
