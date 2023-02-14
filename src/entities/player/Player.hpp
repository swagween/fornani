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
const float JUMPBOX_HEIGHT = 2.0f;
const int JUMP_BUFFER_TIME = 6;

struct PhysicsStats {
    
    float PLAYER_MAX_XVEL = 2.380f;
    float PLAYER_MAX_YVEL = 0.0f;
    
    float AIR_MULTIPLIER = 3.6f;
    
    float PLAYER_GRAV = 0.347f;

    float PLAYER_FRIC = 0.994f;
    float PLAYER_VERT_FRIC = 0.77f;
    float PLAYER_HORIZ_FRIC = 0.77;
    float PLAYER_VERT_AIR_FRIC = 0.973f;
    float PLAYER_HORIZ_AIR_FRIC = 0.973f;
    float PLAYER_MASS = 1.0f;

    float X_ACC = 0.794f;
    float Y_ACC = 0.794f;
    
    float JUMP_ACC = 0.8f;
    float JUMP_MAX = 5.674f;
    int   JUMP_TIME = 20;
    
    void load_from_json(std::string path) {
        
    }
    
    void save_to_json(std::string path) {
        
    }
    
};

class Player {
public:
    
    using Time = std::chrono::duration<float>;
    
    Player();
    
    //member functions
    void handle_events(sf::Event event);
    void update(Time dt);
    void render();
    
    void sync_components();
    void set_position(sf::Vector2<float> new_pos);
    
    
    //member vars
    Shape hurtbox{};
    Shape predictive_hurtbox{};
    Shape jumpbox{};
    
    components::PhysicsComponent physics{};
    PhysicsStats stats{};
    
    bool move_left{};
    bool move_right{};
    bool move_up{};
    bool move_down{};
    
    bool grav = true;
    bool just_collided = false;
    bool is_colliding_with_level{};
    bool has_left_collision{};
    bool has_right_collision{};
    bool grounded = false;
    bool jump_hold = false;
    bool can_jump{};
    bool just_jumped{};
    int jump_request{};
    
    int jump_height_counter{};
    
};
 /* Player_hpp */
