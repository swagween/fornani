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

struct PlayerBehavior {
    
    std::unique_ptr<Behavior> idle = std::make_unique<Behavior>();
    
//    Behavior running =                  Behavior(BEHAVIOR::RUNNING,                 {BEHAVIOR::TURNING, BEHAVIOR::RUNNING, BEHAVIOR::IDLE, BEHAVIOR::JUMPSQUAT},                    44);
//    Behavior jumpsquat =                Behavior(BEHAVIOR::JUMPSQUAT,               {BEHAVIOR::JUMPING_FORWARDS, BEHAVIOR::JUMPING_BACKWARDS},                                      60);
//    Behavior jumping_forwards =         Behavior(BEHAVIOR::JUMPING_FORWARDS,        {BEHAVIOR::DOUBLE_JUMPING_FORWARDS, BEHAVIOR::DOUBLE_JUMPING_BACKWARDS, BEHAVIOR::SUSPENDED},   54);
//    Behavior jumping_backwards =        Behavior(BEHAVIOR::JUMPING_BACKWARDS,       {BEHAVIOR::DOUBLE_JUMPING_FORWARDS, BEHAVIOR::DOUBLE_JUMPING_BACKWARDS, BEHAVIOR::SUSPENDED},   0);//todo
//    Behavior susppended =               Behavior(BEHAVIOR::SUSPENDED,               {BEHAVIOR::FALLING}, 30);
//    Behavior falling =                  Behavior(BEHAVIOR::FALLING,                 {BEHAVIOR::LANDING}, 62);
//    Behavior landing =                  Behavior(BEHAVIOR::LANDING,                 {BEHAVIOR::IDLE}, 56);
//    Behavior inspecting =               Behavior(BEHAVIOR::INSPECTING,              {BEHAVIOR::IDLE}, 0);//todo
//    Behavior turning =                  Behavior(BEHAVIOR::TURNING,                 {BEHAVIOR::IDLE}, 0);//todo
//    Behavior stopping =                 Behavior(BEHAVIOR::STOPPING,                {BEHAVIOR::IDLE}, 70);
//    Behavior hurt =                     Behavior(BEHAVIOR::HURT,                    {BEHAVIOR::NONE}, 0);//todo
//    Behavior dashing =                  Behavior(BEHAVIOR::DASHING,                 {BEHAVIOR::NONE}, 0);//todo
//    Behavior double_jumping_forwards =  Behavior(BEHAVIOR::DOUBLE_JUMPING_FORWARDS, {BEHAVIOR::NONE}, 0);//todo
//    Behavior double_jumping_backwards = Behavior(BEHAVIOR::DOUBLE_JUMPING_BACKWARDS,{BEHAVIOR::NONE}, 0);//todo
//    Behavior yawning =                  Behavior(BEHAVIOR::YAWNING,                 {BEHAVIOR::NONE}, 0);//todo
//    Behavior fidgeting =                Behavior(BEHAVIOR::FIDGETING,               {BEHAVIOR::NONE}, 0);//todo
//    Behavior sleeping =                 Behavior(BEHAVIOR::SLEEPING,                {BEHAVIOR::NONE}, 0);//todo
//    Behavior dying =                    Behavior(BEHAVIOR::DYING,                   {BEHAVIOR::NONE}, 0);//todo
    
    
};


} // end behavior

/* Behavior_hpp */
