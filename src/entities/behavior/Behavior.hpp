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

const int DEFAULT_FRAMERATE = 8;
const int DEFAULT_DURATION = 8;
const int DEFAULT_NUM_BEHAVIORS = 1;

static bool trigger{};

struct BehaviorParameters {
    BehaviorParameters() { set_params(); }
    BehaviorParameters(std::string id, int d, int f, bool t, bool r, bool nl, int lookup) : behavior_id(id), duration(d), framerate(f), transitional(t), restrictive(r), no_loop(nl), lookup_value(lookup) { set_params(); }
    ~BehaviorParameters() { trigger = true; }
    void set_params() {current_frame = 0; anim_frame = framerate - 1; complete = false;}
    const int framerate{};
    int current_frame{};
    int anim_frame{};
    const int duration{};
    int lookup_value{};
    bool no_loop{};
    bool restrictive{};
    bool transitional{};
    bool complete = false;
    std::string behavior_id{};
};

class Behavior {
public:
    Behavior() = default;
    Behavior(BehaviorParameters p) : params(p) { update(); }
    Behavior(const Behavior& b) {}
    Behavior& operator=(Behavior&&) = delete;
    Behavior(Behavior&&) = default;
    
    void refresh() {
        if(params.current_frame >= params.duration) {
            params.current_frame = 0;
            if(params.transitional || params.restrictive) { params.complete = true; }
        }
    }
    
    void update() {
        params.anim_frame--;
        if(params.anim_frame < 0) {
            params.anim_frame = params.framerate - 1;
        }
        if(params.anim_frame == 0) {
            params.current_frame++;
        }
        refresh();
    }
    
    BehaviorParameters params{};
};


} // end behavior

/* Behavior_hpp */
