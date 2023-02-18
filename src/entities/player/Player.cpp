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
    anchor_point = {physics.position.x + PLAYER_WIDTH/2, physics.position.y + PLAYER_HEIGHT/2};
    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
    
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
        if (event.key.code == sf::Keyboard::Up) {
            look_up = true;
        }
        if (event.key.code == sf::Keyboard::Down) {
            look_down = true;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Left) {
            move_left = false;
            has_left_collision = false;
            stopping = true;
            last_dir = behavior::DIR::LEFT;
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = false;
            has_right_collision = false;
            stopping = true;
            last_dir = behavior::DIR::RIGHT;
        }
        if (event.key.code == sf::Keyboard::Up) {
            look_up = false;
        }
        if (event.key.code == sf::Keyboard::Down) {
            look_down = false;
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
    
    if(move_left && move_right) {
        physics.acceleration.x = 0.0f;
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
    
    update_behavior();
}

void Player::render() {
    
}

void Player::sync_components() {
    hurtbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    predictive_hurtbox.update(physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    jumpbox.update(physics.position.x, physics.position.y + PLAYER_HEIGHT, PLAYER_WIDTH, JUMPBOX_HEIGHT);
    left_detector.update(physics.position.x - DETECTOR_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
    right_detector.update(physics.position.x + PLAYER_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
    anchor_point = {physics.position.x + PLAYER_WIDTH/2, physics.position.y + PLAYER_HEIGHT/2};
}

void Player::update_behavior() {
    
    if(!(behavior.current_state.get()->params.transitional && !behavior.current_state.get()->params.complete)) {
        
        if(just_jumped) {
            behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::jumpsquat));
        }
        
        if(move_left || move_right) {
            switch(facing) {
                case behavior::DIR::UP_RIGHT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_up));
                    break;
                case behavior::DIR::DOWN_RIGHT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_down));
                    break;
                case behavior::DIR::UP_LEFT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_up));
                    break;
                case behavior::DIR::DOWN_LEFT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running_down));
                    break;
                default:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::running));
                    break;
            }
        }
        
        if(!grounded) {
            if(physics.velocity.y < 4.0f) {
                behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::rising));
            } else if(physics.velocity.y > 4.0f) {
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::falling));
            } else {
                behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::suspended));
            }
        }
        
        if(grounded && stopping) {
            switch(facing) {
                case behavior::DIR::UP_RIGHT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_up));
                    break;
                case behavior::DIR::DOWN_RIGHT:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop_down));
                    break;
                default:
                    behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::stop));
                    break;
            }
        }
        
        if(facing == behavior::DIR::LEFT || facing == behavior::DIR::UP_LEFT || facing == behavior::DIR::DOWN_LEFT) {
            int lookup = behavior.current_state.get()->params.lookup_value;
            behavior.current_state.get()->params.lookup_value = 210 + (lookup % 10) - (lookup - (lookup % 10));
        }
    }
    
    if(behavior.current_state) {
        behavior.current_state->update(anim_frame);
    }
    
    if(behavior.current_state.get()->params.complete) {
        behavior.current_state = std::move(std::make_unique<behavior::Behavior>(behavior::idle));
    }
    
    anim_frame++;
    update_direction();
    
    stopping = false;
}

void Player::set_position(sf::Vector2<float> new_pos) {
    physics.position = new_pos;
    sync_components();
}

void Player::update_direction() {
    facing = last_dir;
    if(move_right) {
        facing = behavior::DIR::RIGHT;
        if(look_up) {
            facing = behavior::DIR::UP_RIGHT;
        }
        if(look_down) {
            facing = behavior::DIR::DOWN_RIGHT;
        }
    }
    if(move_left) {
        facing = behavior::DIR::LEFT;
        if(look_up) {
            facing = behavior::DIR::UP_LEFT;
        }
        if(look_down) {
            facing = behavior::DIR::DOWN_LEFT;
        }
    }
    if(!move_left && !move_right && look_up) {
        facing = behavior::DIR::UP;
    }
    if(!move_left && !move_right && look_down) {
        facing = behavior::DIR::DOWN;
    }
}

void Player::handle_map_collision(const Shape &cell, bool is_ramp) {
    
    if(left_detector.SAT(cell) && physics.velocity.x < 0.01f && !is_ramp) {
        has_left_collision = true;
        physics.acceleration.x = 0.0f;
        physics.velocity.x = 0.0f;
        left_aabb_counter++;
    }
    if(right_detector.SAT(cell) && physics.velocity.x > -0.01f && !is_ramp) {
        has_right_collision = true;
        physics.acceleration.x = 0.0f;
        physics.velocity.x = 0.0f;
        right_aabb_counter++;
    }
    if(predictive_hurtbox.SAT(cell)) {
        is_any_colllision = true;
        //set mtv
        physics.mtv = predictive_hurtbox.testCollisionGetMTV(predictive_hurtbox, cell);
        
        
        if(physics.velocity.y > 3.0f) {
            physics.mtv.x = 0.0f;
        }
        //here, we can do MTV again with the player's predicted position based on velocity
        if(physics.velocity.y > -0.01f) {
            float ydist = predictive_hurtbox.shape_y - physics.position.y;
            float correction = ydist + physics.mtv.y;
            physics.position.y += correction;
            physics.velocity.y = 0.0f;
            physics.acceleration.y = 0.0f;
        }
        
        sync_components();
        
        //only for landing
        if(physics.velocity.y > 0.0f && !has_left_collision && !has_right_collision) {
            physics.acceleration.y = 0.0f;
            physics.velocity.y = 0.0f;
        }
        //player hits the ceiling
        if(physics.velocity.y < -0.01f && abs(physics.mtv.x) < abs(physics.mtv.y)) {
            physics.acceleration.y = 0.0f;
            physics.velocity.y *= -1;
            physics.mtv.y *= -1;
            jump_hold = false;
        }
        physics.mtv = {0.0f, 0.0f};
        just_collided = true;
        is_colliding_with_level = true;
        
    }
    if(jumpbox.SAT(cell)) {
        is_any_jump_colllision = true;
    }
}

std::string Player::print_direction() {
    switch(facing) {
        case behavior::DIR::NEUTRAL:
            return "NEUTRAL";
            break;
        case behavior::DIR::LEFT:
            return "LEFT";
            break;
        case behavior::DIR::RIGHT:
            return "RIGHT";
            break;
        case behavior::DIR::UP:
            return "UP";
            break;
        case behavior::DIR::DOWN:
            return "DOWN";
            break;
        case behavior::DIR::UP_RIGHT:
            return "UP RIGHT";
            break;
        case behavior::DIR::UP_LEFT:
            return "UP LEFT";
            break;
        case behavior::DIR::DOWN_RIGHT:
            return "DOWN RIGHT";
            break;
        case behavior::DIR::DOWN_LEFT:
            return "DOWN LEFT";
            break;
    }
}
