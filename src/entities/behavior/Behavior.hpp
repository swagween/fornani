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
#include <chrono>

namespace behavior {

const int DEFAULT_FRAMERATE = 8;
const int DEFAULT_DURATION = 8;
const int DEFAULT_NUM_BEHAVIORS = 1;
const int animation_multiplier = 32;

struct BehaviorParameters {
    BehaviorParameters() { set_params(); }
    BehaviorParameters(std::string id, int d, int f, bool t, bool r, bool nl, int lookup) : behavior_id(id), duration(d), framerate(f), transitional(t), restrictive(r), no_loop(nl), lookup_value(lookup) { set_params(); }
    ~BehaviorParameters() {}
    void set_params() {current_frame = 0; anim_frame = framerate - 1; complete = false;}
    int framerate{};
    int current_frame{};
    int anim_frame{};
    int duration{};
    int lookup_value{};
    bool no_loop{};
    bool restrictive{};
    bool transitional{};
    bool complete = false;
    bool done = false;
    bool started = true;
    bool frame_trigger{};
    std::string behavior_id{};
};

class Behavior {

    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;

public:
    Behavior() = default;
    Behavior(BehaviorParameters p) : params(p) { update(); }
    
    void refresh();
    void update();
    int get_frame();

    bool restricted();
    
    BehaviorParameters params{};

    //fixed animation time step variables
    Time dt{ 0.001f };
    Clock::time_point current_time = Clock::now();
    Time accumulator{ 0.0f };
};


} // end behavior

/* Behavior_hpp */
