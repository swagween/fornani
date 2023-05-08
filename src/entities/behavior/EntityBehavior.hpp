//
//  EntityBehavior.hpp
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

enum class DIR_LR {
    LEFT, RIGHT
};

/* BehaviorParameters(id, duration, framerate, transitional, restrictive, no_loop, lookup) */

inline BehaviorParameters idle =            BehaviorParameters("idle", 8, 14, false, false, false, 20);
inline BehaviorParameters idle_up =         BehaviorParameters("idle_up", 8, DEFAULT_FRAMERATE, false, false, false, 0);
inline BehaviorParameters idle_down =       BehaviorParameters("idle_down", 8, DEFAULT_FRAMERATE, false, false, false, 10);
inline BehaviorParameters jumpsquat =       BehaviorParameters("jumpsquat", 2, 4, true, true, true, 60);
inline BehaviorParameters rising =          BehaviorParameters("rising", 2, DEFAULT_FRAMERATE, true, false, false, 54);
inline BehaviorParameters suspended =       BehaviorParameters("suspended", 3, 10, false, false, false, 30);
inline BehaviorParameters falling =         BehaviorParameters("falling", 4, 10, false, false, false, 62);
inline BehaviorParameters running_up =      BehaviorParameters("running_up", 4, 10, false, false, false, 40);
inline BehaviorParameters running_down =    BehaviorParameters("running_down", 4, 10, false, false, false, 50);
inline BehaviorParameters running =         BehaviorParameters("running", 4, 10, false, false, false, 44);
inline BehaviorParameters landing =         BehaviorParameters("landing", 2, 8, true, false, true, 56);
inline BehaviorParameters stop =            BehaviorParameters("stop", 1, 12, true, false, true, 75);
inline BehaviorParameters stop_up =         BehaviorParameters("stop_up", 2, DEFAULT_FRAMERATE, false, false, true, 70);
inline BehaviorParameters stop_down =       BehaviorParameters("stop_down", 2, DEFAULT_FRAMERATE, false, false, true, 72);
inline BehaviorParameters turning =         BehaviorParameters("turning", 2, 6, true, true, true, 33);
inline BehaviorParameters inspecting =      BehaviorParameters("inspecting", 2, 12, false, false, true, 37);
inline BehaviorParameters wall_sliding =    BehaviorParameters("wall_sliding", 4, DEFAULT_FRAMERATE, false, false, false, 66);
inline BehaviorParameters hurt =            BehaviorParameters("hurt", 2, 12, true, true, true, 76);

inline BehaviorParameters frdog_idle =      BehaviorParameters("frdog_idle", 2, 12, false, false, false, 0);
inline BehaviorParameters frdog_asleep =    BehaviorParameters("frdog_asleep", 2, 64, true, false, false, 11);
inline BehaviorParameters frdog_charging =  BehaviorParameters("frdog_charging", 2, DEFAULT_FRAMERATE, false, false, false, 5);
inline BehaviorParameters frdog_bite =      BehaviorParameters("frdog_bite", 2, DEFAULT_FRAMERATE, false, false, true, 7);
inline BehaviorParameters frdog_hurt =      BehaviorParameters("frdog_hurt", 2, DEFAULT_FRAMERATE, false, false, false, 9);
inline BehaviorParameters frdog_awakened =  BehaviorParameters("frdog_awakened", 2, 12, true, false, true, 2);
inline BehaviorParameters frdog_bark =      BehaviorParameters("frdog_bark", 1, 16, true, false, true, 15);
inline BehaviorParameters frdog_turn =      BehaviorParameters("frdog_turn", 1, 12, true , true, true, 13);
inline BehaviorParameters frdog_run =       BehaviorParameters("frdog_run", 4, DEFAULT_FRAMERATE, false, false, false, 16);

inline BehaviorParameters hulmet_idle =     BehaviorParameters("hulmet_idle", 1, 12, false, false, false, 7);
inline BehaviorParameters hulmet_run =      BehaviorParameters("hulmet_run", 4, 12, false, false, false, 7);
inline BehaviorParameters hulmet_charging = BehaviorParameters("hulmet_charging", 2, 12, false, false, false, 4);
inline BehaviorParameters hulmet_alert =    BehaviorParameters("hulmet_alert", 1, 12, false, false, false, 6);
inline BehaviorParameters hulmet_hurt =     BehaviorParameters("hulmet_hurt", 4, 12, false, false, false, 0);
inline BehaviorParameters hulmet_dying =    BehaviorParameters("hulmet_dying", 1, 12, false, false, false, 11);
inline BehaviorParameters hulmet_turn =     BehaviorParameters("hulmet_turn", 1, 12, true, true, true, 13);


} // end behavior

/* EntityBehavior_hpp */
