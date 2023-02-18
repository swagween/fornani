//
//  Behavior.hpp
//  behavior
//
//

#pragma once

#include "Behavior.hpp"

namespace behavior {

enum class DIR {
    NEUTRAL,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    UP_RIGHT,
    UP_LEFT,
    DOWN_RIGHT,
    DOWN_LEFT
};

static BehaviorParameters idle = BehaviorParameters("idle", 8, false, 20);
static BehaviorParameters idle_up = BehaviorParameters("idle_up", 8, false, 0);
static BehaviorParameters idle_down = BehaviorParameters("idle_down", 8, false, 10);
static BehaviorParameters jumpsquat = BehaviorParameters("jumpsquat", 2, true, 60);
static BehaviorParameters rising = BehaviorParameters("rising", 2, true, 54);
static BehaviorParameters suspended = BehaviorParameters("suspended", 3, false, 30);
static BehaviorParameters falling = BehaviorParameters("falling", 4, false, 62);
static BehaviorParameters running_up = BehaviorParameters("running_up", 4, false, 40);
static BehaviorParameters running_down = BehaviorParameters("running_down", 4, false, 50);
static BehaviorParameters running = BehaviorParameters("running", 4, false, 44);
static BehaviorParameters landing = BehaviorParameters("landing", 2, true, 56);
static BehaviorParameters stop = BehaviorParameters("stop", 2, true, 74);
static BehaviorParameters stop_up = BehaviorParameters("stop_up", 2, true, 70);
static BehaviorParameters stop_down = BehaviorParameters("stop_down", 2, true, 72);


struct PlayerBehavior {
    PlayerBehavior() {}
};


} // end behavior

/* Behavior_hpp */
