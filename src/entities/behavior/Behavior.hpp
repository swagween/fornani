//
//  Behavior.hpp
//  behavior
//
//

#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <list>

namespace behavior {

const int DEFAULT_FRAMERATE = 4;
const int DEFAULT_DURATION = 8;
const int DEFAULT_NUM_BEHAVIORS = 1;

class Behavior {
public:
    Behavior() = default;
    Behavior(int id, int key) : behavior_id(id), lookup_value(key) {};
    Behavior(const Behavior& b) {}
    Behavior& operator=(Behavior&&) = delete;
    
    void update(int frame) {
        if(frame % framerate == 0) {
            current_frame++;
        }
        if(current_frame > duration) {
            current_frame = 1;
            if(no_loop) { complete = true; }
        }
    }
    
    int framerate{};
    int current_frame{};
    int duration{};
    int lookup_value{};
    bool no_loop{};
    bool must_terminate{};
    bool complete = false;;
    int behavior_id{};
};


} // end behavior

/* Behavior_hpp */
