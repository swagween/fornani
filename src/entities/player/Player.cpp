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
    predictive_hurtbox.init();
    jumpbox.init();
    
    hurtbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y);
    hurtbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y);
    hurtbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT);
    hurtbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT);
    
    predictive_hurtbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y);
    predictive_hurtbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y);
    predictive_hurtbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT);
    predictive_hurtbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT);
    
    jumpbox.vertices[0] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT - JUMPBOX_HEIGHT);
    jumpbox.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH, PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    jumpbox.vertices[3] = sf::Vector2<float>(PLAYER_START_X,  PLAYER_START_Y + PLAYER_HEIGHT + JUMPBOX_HEIGHT);
    
    left_detector.vertices[0] = sf::Vector2<float>(PLAYER_START_X - DETECTOR_WIDTH,  PLAYER_START_Y + DETECTOR_BUFFER);
    left_detector.vertices[1] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + DETECTOR_BUFFER);
    left_detector.vertices[2] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + DETECTOR_HEIGHT);
    left_detector.vertices[3] = sf::Vector2<float>(PLAYER_START_X - DETECTOR_WIDTH,  PLAYER_START_Y + DETECTOR_HEIGHT);
    
    right_detector.vertices[0] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH,  PLAYER_START_Y + DETECTOR_BUFFER);
    right_detector.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH, PLAYER_START_Y + DETECTOR_BUFFER);
    right_detector.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH, PLAYER_START_Y + DETECTOR_HEIGHT);
    right_detector.vertices[3] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH,  PLAYER_START_Y + DETECTOR_HEIGHT);
    
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
            has_left_collision = false;
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = false;
            has_right_collision = false;
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Z) {
            jump_request = JUMP_BUFFER_TIME;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            jump_hold = false;
            can_jump = true;
        }
    }
}

void Player::update(Time dt) {
    
    //check if player requested jump
    if(grounded && jump_request > 0) {
        physics.velocity.y = 0.0f;
        jump_hold = true;
        just_jumped = true;
        jump_height_counter = 0;
    }
    
    
    //check keystate
    if(move_left && !has_left_collision) {
        if(grounded) {
            physics.acceleration.x = -stats.X_ACC;
        } else {
            physics.acceleration.x = -stats.X_ACC/stats.AIR_MULTIPLIER;
        }
    }
    if(move_right && !has_right_collision) {
        if(grounded) {
            physics.acceleration.x = stats.X_ACC;
        } else {
            physics.acceleration.x = stats.X_ACC/stats.AIR_MULTIPLIER;
        }
    }
    //zero the player's horizontal acceleration if movement was not requested
    if((!move_left && !move_right)) {
        physics.acceleration.x = 0.0f;
    }
    
    //gravity and stats corrections
    if(!grounded) {
        physics.acceleration.y += stats.PLAYER_GRAV;
    }
    
    
//    if(abs(physics.mtv.y) > 0.01) {
//        physics.acceleration.y = 0.0f;
//    }
//    if(grounded) {
//        physics.acceleration.y = 0.0f;
//        physics.velocity.y = 0.0f;
//    }
    
    
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
    
    //now jump after all the y corrections
    if(jump_height_counter < stats.JUMP_TIME && jump_hold) {
        physics.acceleration.y = -stats.JUMP_MAX;
        just_jumped = false;
        ++jump_height_counter;
        can_jump = false;
    }
    
    if(jump_request > -1) {
        jump_request--;
    }
    
    
    physics.update_euler(dt);
    
    
    sync_components();
    
    //for parameter tweaking, remove later
    if(grounded) {
        physics.friction = {stats.PLAYER_HORIZ_FRIC, stats.PLAYER_VERT_FRIC};
    } else {
        physics.friction = {stats.PLAYER_HORIZ_AIR_FRIC, stats.PLAYER_VERT_AIR_FRIC};
    }
    if(!is_colliding_with_level) {
        physics.mtv = {0.0f, 0.0f};
    }
    just_collided = false;
}

void Player::render() {
    
}

void Player::sync_components() {
    hurtbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    predictive_hurtbox.update(physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    jumpbox.update(physics.position.x, physics.position.y + PLAYER_HEIGHT, PLAYER_WIDTH, JUMPBOX_HEIGHT);
    left_detector.update(physics.position.x - DETECTOR_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
    right_detector.update(physics.position.x + PLAYER_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
}

void Player::set_position(sf::Vector2<float> new_pos) {
    physics.position = new_pos;
    sync_components();
}
