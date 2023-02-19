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

/* BehaviorParameters(id, duration, framerate, transitional, restrictive, lookup) */

static BehaviorParameters idle =            BehaviorParameters("idle", 8, DEFAULT_FRAMERATE, false, false, false, 20);
static BehaviorParameters idle_up =         BehaviorParameters("idle_up", 8, DEFAULT_FRAMERATE, false, false, false, 0);
static BehaviorParameters idle_down =       BehaviorParameters("idle_down", 8, DEFAULT_FRAMERATE, false, false, false, 10);
static BehaviorParameters jumpsquat =       BehaviorParameters("jumpsquat", 2, 4, true, true, true, 60);
static BehaviorParameters rising =          BehaviorParameters("rising", 2, DEFAULT_FRAMERATE, true, false, false, 54);
static BehaviorParameters suspended =       BehaviorParameters("suspended", 3, DEFAULT_FRAMERATE, false, false, false, 30);
static BehaviorParameters falling =         BehaviorParameters("falling", 4, DEFAULT_FRAMERATE, false, false, false, 62);
static BehaviorParameters running_up =      BehaviorParameters("running_up", 4, DEFAULT_FRAMERATE, false, false, false, 40);
static BehaviorParameters running_down =    BehaviorParameters("running_down", 4, DEFAULT_FRAMERATE, false, false, false, 50);
static BehaviorParameters running =         BehaviorParameters("running", 4, DEFAULT_FRAMERATE, false, false, false, 44);
static BehaviorParameters landing =         BehaviorParameters("landing", 2, 8, true, false, true, 56);
static BehaviorParameters stop =            BehaviorParameters("stop", 2, DEFAULT_FRAMERATE, false, false, true, 74);
static BehaviorParameters stop_up =         BehaviorParameters("stop_up", 2, DEFAULT_FRAMERATE, false, false, true, 70);
static BehaviorParameters stop_down =       BehaviorParameters("stop_down", 2, DEFAULT_FRAMERATE, false, false, true, 72);
static BehaviorParameters turning =         BehaviorParameters("turning", 2, 4, true, true, true, 33);
static BehaviorParameters wall_sliding =    BehaviorParameters("wall_sliding", 4, DEFAULT_FRAMERATE, false, false, false, 66);



} // end behavior

/* Behavior_hpp */
