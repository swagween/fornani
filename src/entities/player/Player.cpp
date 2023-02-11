//
//  Player.cpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#include "Player.hpp"

Player::Player() {
    
    physics = components::PhysicsComponent({stats.PLAYER_HORIZ_FRIC, stats.PLAYER_VERT_FRIC}, stats.PLAYER_MASS);
    
    hurtbox.init();
    jumpbox.init();
    
    hurtbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y);
    hurtbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y);
    hurtbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT);
    hurtbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT);
    
    jumpbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    jumpbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    
}

void Player::handle_events(sf::Event event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left) {
            move_left = true;
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = true;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Left) {
            move_left = false;
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = false;
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Z) {
            if(grounded) {
                jump_hold = true;
                just_jumped = true;
                jump_height_counter = 0;
                
            }
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            jump_hold = false;
            
        }
    }
}

void Player::update(Time dt) {
    
    //check keystate
    if(move_left) {
        physics.acceleration.x = -stats.X_ACC;
    }
    if(move_right) {
        physics.acceleration.x = stats.X_ACC;
    }
    if(!move_left && !move_right) {
        physics.acceleration.x = 0.0f;
    }
    
    if(jump_height_counter < stats.JUMP_TIME) {
        physics.acceleration.y = -stats.JUMP_MAX;
        just_jumped = false;
        ++jump_height_counter;
    }
    
    sf::operator+=(physics.position, physics.mtv);
    
    if(!grounded) {
        physics.acceleration.y += stats.PLAYER_GRAV;
    }
    
    if(abs(physics.mtv.y) > 1.9) {
        physics.acceleration.y = 0.0f;
    }
    
    if(grounded) {
        physics.velocity.y = 0.0f;
    }
    physics.mtv = {0.0f, 0.0f};
    just_collided = false;
    
    //impose physics limitations
    if(physics.velocity.x > stats.PLAYER_MAX_XVEL) {
        physics.velocity.x = stats.PLAYER_MAX_XVEL;
    }
    if(physics.velocity.x < -stats.PLAYER_MAX_XVEL) {
        physics.velocity.x = -stats.PLAYER_MAX_XVEL;
    }
    if(physics.velocity.y > stats.PLAYER_MAX_YVEL) {
        physics.velocity.y = stats.PLAYER_MAX_YVEL;
    }
    if(physics.velocity.y < -stats.PLAYER_MAX_YVEL) {
        physics.velocity.y = -stats.PLAYER_MAX_YVEL;
    }
    
    physics.update_euler(dt);
    
    sync_components();
    
    //for parameter tweaking, remove later
    physics.friction = {stats.PLAYER_HORIZ_FRIC, stats.PLAYER_VERT_FRIC};
}

void Player::render() {
    
}

void Player::sync_components() {
    hurtbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    jumpbox.update(physics.position.x, physics.position.y + PLAYER_HEIGHT, PLAYER_WIDTH, JUMPBOX_HEIGHT);
}

void Player::set_position(sf::Vector2<float> new_pos) {
    physics.position = new_pos;
    sync_components();
}
