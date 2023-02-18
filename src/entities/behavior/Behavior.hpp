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

const int DEFAULT_FRAMERATE = 20;
const int DEFAULT_DURATION = 8;
const int DEFAULT_NUM_BEHAVIORS = 1;

struct BehaviorParameters {
    BehaviorParameters() { set_params(false); }
    BehaviorParameters(std::string id, int d, bool t, int lookup) : behavior_id(id), duration(d), transitional(t), lookup_value(lookup) { set_params(t); }
    void set_params(bool t) { must_terminate = t; no_loop = t; framerate = DEFAULT_FRAMERATE; current_frame = 0; complete = false;}
    int framerate{};
    int current_frame{};
    int duration{};
    int lookup_value{};
    bool no_loop{};
    bool must_terminate{};
    bool transitional{};
    bool complete = false;
    std::string behavior_id{};
};

class Behavior {
public:
    Behavior() = default;
    Behavior(BehaviorParameters p) : params(p) {}
    Behavior(const Behavior& b) {
        params = b.params;
    }
    Behavior& operator=(Behavior&&) = delete;
    Behavior(Behavior&&) = default;
    
    void update(int frame) {
        if(frame % params.framerate == 0) {
            params.current_frame++;
        }
        if(params.current_frame >= params.duration) {
            params.current_frame = 0;
            if(params.no_loop) { params.complete = true; }
        }
    }
    
    void set_params(BehaviorParameters& p) {
        params = p;
    }
    
    BehaviorParameters params{};
};


} // end behavior

/* Behavior_hpp */
