//
//  Animation.hpp
//  anim
//
//

#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <list>
#include <chrono>

namespace anim {

const int DEFAULT_FRAMERATE = 8;
const int DEFAULT_DURATION = 8;
const int DEFAULT_NUM_BEHAVIORS = 1;
const int animation_multiplier = 32;

struct AnimationParameters {
    AnimationParameters() { set_params(); }
    AnimationParameters(int d, int f, bool nl) :  duration(d), framerate(f), no_loop(nl) { set_params(); }
    ~AnimationParameters() {}
    void set_params() { current_frame = 0; anim_frame = framerate - 1; started = true; }
    int framerate{};
    int current_frame{};
    int anim_frame{};
    int duration{};
    bool no_loop{};
    bool done{ false };
    bool started{ true };
    bool frame_trigger{};
};

class Animation {

    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;

public:
    Animation() = default;
    Animation(AnimationParameters p) : params(p) { update(); }
    
    void refresh();
    void start();
    void update();
    void end(bool cutoff);
    int get_frame();
    
    AnimationParameters params{};

    //fixed animation time step variables
    Time dt{ 0.001f };
    Clock::time_point current_time = Clock::now();
    Time accumulator{ 0.0f };
};


} // end anim

/* Animation_hpp */
