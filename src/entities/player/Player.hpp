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

class Player {
public:
    
    float PLAYER_MAX_XVEL = 2.0f;
    float PLAYER_MAX_YVEL = 8.0f;

    float PLAYER_FRIC = 0.9f;
    float PLAYER_MASS = 1.0f;

    float X_ACC = 0.5f;
    float Y_ACC = 3.0f;

    float PLAYER_GRAV = 0.8f;
    
    Player();
    
    void handle_events(sf::Event event);
    void update();
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
    
    bool grav{};
    
    sf::Vector2<float> mtv{};
    
};
 /* Player_hpp */
