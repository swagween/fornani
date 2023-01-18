//
//  Player.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//
#pragma once

#include "../../utils/Shape.hpp"
#include "../../components/PhysicsComponent.hpp"

const float PLAYER_WIDTH = 32.0f;
const float PLAYER_HEIGHT = 32.0f;
const float PLAYER_START_X = 100.0f;
const float PLAYER_START_Y = 100.0f;
const float JUMPBOX_HEIGHT = 1.0f;

class Player {
public:
    
    using Time = std::chrono::duration<float>;
    
    float PLAYER_MAX_XVEL = 5.8f;
    float PLAYER_MAX_YVEL = 8.0f;
    
    float PLAYER_GRAV = 0.696f;

    float PLAYER_FRIC = 0.694f;
    float PLAYER_AIR_FRIC = 0.745f;
    float PLAYER_VERT_FRIC = 0.443f;
    float PLAYER_MASS = 1.0f;

    float X_ACC = 0.412f;
    float AIR_X_ACC = 0.412;
    float Y_ACC = 3.9f;
    
    float JUMP_ACC = 0.8f;
    float JUMP_MAX = 21.443f;
    int JUMP_TIME = 15;

    
    
    Player();
    
    void handle_events(sf::Event event);
    void update(Time dt);
    void render();
    
    void sync_components();
    void set_position(sf::Vector2<float> new_pos);
    
    Shape hurtbox{};
    Shape jumpbox{};
    
    components::PhysicsComponent physics{};
    bool move_left{};
    bool move_right{};
    bool move_up{};
    bool move_down{};
    
    int jump_height_counter{};
    
    bool grav = true;
    bool just_collided = false;
    bool is_colliding_with_level{};
    bool grounded = false;
    bool jump_hold = false;
    
    sf::Vector2<float> mtv{};
    
};
 /* Player_hpp */
