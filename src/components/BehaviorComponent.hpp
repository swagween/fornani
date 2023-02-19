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
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
            flip_left();
        }
    }
    
    void end_loop() {
        if(current_state.get()->params.complete && current_state.get()->params.no_loop) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
            flip_left();
        }
    }
    
    void reset() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
            flip_left();
        }
    }
    
    void run() {
        if(ready()) {
            switch(facing) {
                case behavior::DIR::UP_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_up));
                    flip_left();
                    break;
                case behavior::DIR::DOWN_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_down));
                    flip_left();
                    break;
                case behavior::DIR::UP_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_up));
                    flip_left();
                    break;
                case behavior::DIR::DOWN_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_down));
                    flip_left();
                    break;
                default:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running));
                    flip_left();
                    break;
            }
        }
    }
    
    void stop() {
        if(ready()) {
            switch(facing) {
                case behavior::DIR::UP_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_up));
                    flip_left();
                    break;
                case behavior::DIR::DOWN_RIGHT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_down));
                    flip_left();
                    break;
                case behavior::DIR::UP_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_up));
                    flip_left();
                    break;
                case behavior::DIR::DOWN_LEFT:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_down));
                    flip_left();
                    break;
                default:
                    current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop));
                    flip_left();
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
            flip_left();
        }
    }
    void rise() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::rising));
            flip_left();
        }
    }
    void suspend() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::suspended));
            flip_left();
        }
    }
    void fall() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::falling));
            flip_left();
        }
    }
    
    void land() {
        if(ready()) {
            current_state = std::move(std::make_unique<behavior::Behavior>(behavior::landing));
            flip_left();
        }
    }
    
    void air(float velocity) {
        if(ready()) {
            if(velocity < -suspension_threshold*2) {
                rise();
                flip_left();
            } else if (velocity < suspension_threshold) {
                suspend();
            } else {
                fall();
                flip_left();
            }
        }
    }
    
    void flip_left() {
        if(facing_left()) {
            int lookup = current_state->params.lookup_value;
            if(lookup < 110) {
                current_state->params.lookup_value = 210 + (lookup % 10) - (lookup - (lookup % 10));
            }
        }
    }
    
    bool ready() {
        return !current_state->params.transitional || current_state->params.complete;
    }
    
    bool restricted() {
        return current_state->params.restrictive && !current_state->params.complete;
    }
    
    bool facing_left() {
        return facing == behavior::DIR::LEFT || facing == behavior::DIR::UP_LEFT || facing == behavior::DIR::DOWN_LEFT;
    }
    
    bool facing_right() {
        return facing == behavior::DIR::RIGHT || facing == behavior::DIR::UP_RIGHT || facing == behavior::DIR::DOWN_RIGHT;
    }
    
    const float suspension_threshold = 3.0f;
    behavior::DIR facing{};
    
};

} // end components

/* BehaviorComponent_hpp */
