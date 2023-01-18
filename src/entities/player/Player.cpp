//
//  Player.cpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#include "Player.hpp"

Player::Player() {
    
    physics = components::PhysicsComponent(PLAYER_FRIC, PLAYER_MASS);
    
    hurtbox.init();
    jumpbox.init();
    
    hurtbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y);
    hurtbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y);
    hurtbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT);
    hurtbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + PLAYER_HEIGHT);
    
    jumpbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    jumpbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    
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
            }
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            jump_hold = false;
            jump_height_counter = 0;
        }
    }
}

void Player::update(Time dt) {
    
//    if(!grounded) {
//        physics.acceleration.x = PLAYER_AIR_FRIC;
//    } else {
//        physics.acceleration.x = PLAYER_FRIC;
//    }
    
    
    
    //check keystate
    if(move_left) {
        if(grounded) {
            physics.apply_force({-X_ACC, 0.0f});
        } else {
            physics.apply_force({-AIR_X_ACC, 0.0f});
       }
    }
    if(move_right) {
        if(grounded) {
            physics.apply_force({X_ACC, 0.0f});
        } else {
            physics.apply_force({AIR_X_ACC, 0.0f});
        }
    }
    if(jump_hold) {
        if(jump_height_counter < JUMP_TIME) {
            physics.apply_force({0.0f, -JUMP_MAX});
//            physics.velocity.y = -JUMP_MAX;
            physics.acceleration.y *= PLAYER_VERT_FRIC;
            physics.velocity.y *= PLAYER_VERT_FRIC;
        }
        ++jump_height_counter;
    }
    if(grounded) {
        physics.friction = PLAYER_FRIC;
    } else {
        physics.friction = PLAYER_AIR_FRIC;
        
    }
    
    sf::operator+=(physics.position, mtv);
    sf::operator+=(physics.acceleration, mtv);
    //gravity (off for now)
    if(grav && !grounded) {
        physics.apply_force({0.0f, PLAYER_GRAV});
    }
    mtv = {0.0f, 0.0f};
    just_collided = false;
    
//    if(physics.velocity.y < 0.5) {
//        physics.velocity.y = 0.0f;
//    }
    //impose physics limitations
    if(physics.velocity.x > PLAYER_MAX_XVEL) {
        physics.velocity.x = PLAYER_MAX_XVEL;
    }
    if(physics.velocity.x < -PLAYER_MAX_XVEL) {
        physics.velocity.x = -PLAYER_MAX_XVEL;
    }
    if(physics.velocity.y > PLAYER_MAX_YVEL) {
        physics.velocity.y = PLAYER_MAX_YVEL;
    }
    if(physics.velocity.y < -PLAYER_MAX_YVEL) {
        physics.velocity.y = -PLAYER_MAX_YVEL;
    }
    
    physics.update_euler(dt);
    
    
    
    sync_components();
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
