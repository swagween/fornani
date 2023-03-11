//
//  Player.cpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//

#include "Player.hpp"
#include "../../setup/ServiceLocator.hpp"

Player::Player() {
    
    physics = components::PhysicsComponent({stats.PLAYER_HORIZ_FRIC, stats.PLAYER_VERT_FRIC}, stats.PLAYER_MASS);
    anchor_point = {physics.position.x + PLAYER_WIDTH/2, physics.position.y + PLAYER_HEIGHT/2};
    behavior.current_state = std::make_unique<behavior::Behavior>(behavior::idle);
    behavior.facing_lr = behavior::DIR_LR::RIGHT;
    
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
    
    
    left_detector.left_offset = DETECTOR_WIDTH - 0.0001;
    right_detector.right_offset = DETECTOR_WIDTH - 0.0001;
    
    left_detector.vertices[0] = sf::Vector2<float>(PLAYER_START_X - DETECTOR_WIDTH,  PLAYER_START_Y + DETECTOR_BUFFER);
    left_detector.vertices[1] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + DETECTOR_BUFFER);
    left_detector.vertices[2] = sf::Vector2<float>(PLAYER_START_X, PLAYER_START_Y + DETECTOR_HEIGHT);
    left_detector.vertices[3] = sf::Vector2<float>(PLAYER_START_X + left_detector.left_offset - DETECTOR_WIDTH,  PLAYER_START_Y + DETECTOR_HEIGHT);
    
    right_detector.vertices[0] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH,  PLAYER_START_Y + DETECTOR_BUFFER);
    right_detector.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH, PLAYER_START_Y + DETECTOR_BUFFER);
    right_detector.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH - right_detector.right_offset, PLAYER_START_Y + DETECTOR_HEIGHT);
    right_detector.vertices[3] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH,  PLAYER_START_Y + DETECTOR_HEIGHT);
    
    wall_slide_detector.vertices[0] = sf::Vector2<float>(PLAYER_START_X - DETECTOR_WIDTH,  PLAYER_START_Y + DETECTOR_BUFFER + WALL_SLIDE_DETECTOR_OFFSET);
    wall_slide_detector.vertices[1] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH, PLAYER_START_Y + DETECTOR_BUFFER + WALL_SLIDE_DETECTOR_OFFSET);
    wall_slide_detector.vertices[2] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH + DETECTOR_WIDTH - right_detector.right_offset, PLAYER_START_Y + DETECTOR_HEIGHT/4);
    wall_slide_detector.vertices[3] = sf::Vector2<float>(PLAYER_START_X + PLAYER_WIDTH,  PLAYER_START_Y + DETECTOR_HEIGHT/4);
    
    weapons_hotbar = {
        arms::WEAPON_TYPE::BRYNS_GUN,
        arms::WEAPON_TYPE::PLASMER,
        arms::WEAPON_TYPE::CLOVER
    };
    loadout.equipped_weapon = weapons_hotbar.at(0);
    
}

void Player::handle_events(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left) {
            move_left = true;
            if(grounded) {
                if(behavior.facing == behavior::DIR::RIGHT) {
                    behavior.turn();
                    behavior.facing = behavior::DIR::LEFT;
                    is_wall_sliding = false;
                } else {
                    behavior.run();
                }
            } else {
                if(!is_wall_sliding) {
                    behavior.air(physics.velocity.y);
                }
            }
            behavior.facing = behavior::DIR::LEFT;
            behavior.facing_lr = behavior::DIR_LR::LEFT;
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = true;
            if(grounded) {
                if(behavior.facing == behavior::DIR::LEFT) {
                    behavior.turn();
                    is_wall_sliding = false;
                } else {
                    behavior.run();
                }
            } else {
                if(!is_wall_sliding) {
                    behavior.air(physics.velocity.y);
                }
            }
            behavior.facing = behavior::DIR::RIGHT;
            behavior.facing_lr = behavior::DIR_LR::RIGHT;
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
            if(!has_right_collision) {
                is_wall_sliding = false;
            }
            stopping = true;
            last_dir = behavior::DIR::LEFT;
            left_released = true;
            if(grounded) {
                if(!move_right && !behavior.restricted()) {
                    behavior.reset();
                }
            } else {
                if(!behavior.restricted() && !is_wall_sliding) {
                    behavior.air(physics.velocity.y);
                }
            }
            if(move_right) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = false;
            has_right_collision = false;
            if(!has_left_collision) {
                is_wall_sliding = false;
            }
            stopping = true;
            last_dir = behavior::DIR::RIGHT;
            right_released = true;
            if(grounded) {
                if(!behavior.restricted()) {
                    behavior.reset();
                }
            } else {
                if(!behavior.restricted() && !is_wall_sliding) {
                    behavior.air(physics.velocity.y);
                }
            }
            if(move_left) { behavior.facing_lr = behavior::DIR_LR::LEFT; }
        }
        if (event.key.code == sf::Keyboard::Up) {
            look_up = false;
        }
        if (event.key.code == sf::Keyboard::Down) {
            look_down = false;
        }
        if (event.key.code == sf::Keyboard::X) {
            weapon_fired = false;
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Z) {
            is_jump_pressed = true;
            jump_request = JUMP_BUFFER_TIME;
            just_jumped = true;
            jump_trigger = true;
            if(grounded) {
                soundboard_flags.jump = true;
            }
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            is_jump_pressed = false;
            jump_hold = false;
            if(!behavior.restricted()) {
                can_jump = true;
            }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::X) {
            weapon_fired = true;
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::A) {
            if(!weapons_hotbar.empty()) {
                current_weapon--;
                if(current_weapon < 0) { current_weapon = (int)weapons_hotbar.size() - 1; }
                loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
                soundboard_flags.weapon_swap = true;
            }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::S) {
            if(!weapons_hotbar.empty()) {
                current_weapon++;
                if(current_weapon > weapons_hotbar.size() - 1) { current_weapon = 0; }
                loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
                soundboard_flags.weapon_swap = true;
            }
        }
    }
}

void Player::update(Time dt) {
    if(behavior::trigger) { behavior.reset(); }
    update_animation();
    //check if player requested jump
    if(grounded && jump_request > 0) {
        physics.velocity.y = 0.0f;
        if(!behavior.restricted()) {
            jump_height_counter = 0;
        }
        if(just_jumped) {
            behavior.jump();
            jump_hold = true;
        }
        if(is_jump_pressed) {
        }
    }
    if(!is_jump_pressed) { jump_hold = false; }
    
    //check keystate
    if(!behavior.restricted()) {
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
        if((move_left || move_right) && grounded && abs(physics.velocity.x) > stats.PLAYER_MAX_XVEL) {
//                    svc::assetLocator.get().step.play();
                    // I should do this:
                    //                svc::soundboardLocator.get().play(sb::sfx::STEP);
                    // or this (definitely this):
                    soundboard_flags.step = true;
                
            
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
    
    //weapon physics
    if(weapon_fired) {
        if(behavior.facing == behavior::DIR::LEFT) {
            if(!has_right_collision) {
                physics.apply_force({loadout.get_equipped_weapon().attributes.recoil, -loadout.get_equipped_weapon().attributes.recoil/4});
            }
        }
        if(behavior.facing == behavior::DIR::RIGHT) {
            if(!has_left_collision) {
                physics.apply_force({-loadout.get_equipped_weapon().attributes.recoil, -loadout.get_equipped_weapon().attributes.recoil/4});
            }
        }
    }
    
    //impose physics limitations
    if(!behavior.restricted()) {
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
    }
    
    
    if(is_wall_sliding) {
//        physics.acceleration.y = 0.0f;
//        physics.velocity.y = stats.WALL_SLIDE_SPEED;
    }
    
    //now jump after all the y corrections
    if(jump_height_counter < stats.JUMP_TIME && (is_jump_pressed && jump_hold)) {
        if(!behavior.restricted()) {
            physics.acceleration.y = -stats.JUMP_MAX;
            ++jump_height_counter;
            can_jump = false;
            jump_trigger = false;
            behavior.rise();
        }
    }
    
    if(jump_request > -1) {
        if(!behavior.restricted()) {
            jump_request--;
            //still jump for quick presses
            if(!is_jump_pressed && grounded) {
                physics.acceleration.y = -stats.JUMP_MAX;
                ++jump_height_counter;
                can_jump = false;
                jump_trigger = false;
                jump_request = -1;
                behavior.rise();
            }
        }
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
    apparent_position.x = physics.position.x - (48 - PLAYER_WIDTH)/2;
    apparent_position.y = physics.position.y - (48 - PLAYER_HEIGHT);
    play_sounds();
}

void Player::render() {
    
}

void Player::update_animation() {
    if(behavior.current_state) {
        behavior.end_loop();
        behavior.current_state->update();
    }
}

void Player::sync_components() {
    hurtbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    predictive_hurtbox.update(physics.position.x + physics.velocity.x, physics.position.y + physics.velocity.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    jumpbox.update(physics.position.x, physics.position.y + PLAYER_HEIGHT, PLAYER_WIDTH, JUMPBOX_HEIGHT);
    left_detector.update(physics.position.x - DETECTOR_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
    right_detector.update(physics.position.x + PLAYER_WIDTH, physics.position.y + DETECTOR_BUFFER, DETECTOR_WIDTH, DETECTOR_HEIGHT);
    wall_slide_detector.update(physics.position.x - DETECTOR_WIDTH, physics.position.y + DETECTOR_BUFFER + WALL_SLIDE_DETECTOR_OFFSET, PLAYER_WIDTH + DETECTOR_WIDTH*2, DETECTOR_HEIGHT/4);
    if(behavior.facing_left()) {
        anchor_point = {physics.position.x + PLAYER_WIDTH/2 - ANCHOR_BUFFER, physics.position.y + PLAYER_HEIGHT/2};
    } else if(behavior.facing_right()) {
        anchor_point = {physics.position.x + PLAYER_WIDTH/2 + ANCHOR_BUFFER, physics.position.y + PLAYER_HEIGHT/2};
    } else {
        anchor_point = {physics.position.x + PLAYER_WIDTH/2, physics.position.y + PLAYER_HEIGHT/2};
    }
}

void Player::update_behavior() {
    
//    if(wall_slide_trigger) {
//        behavior.wall_slide();
//    } else if(release_wallslide && behavior.current_state->params.behavior_id == "wall_sliding") {
//        behavior.reset();
//    }
    
    
    
    if(just_jumped && !is_wall_sliding) {
        behavior.air(physics.velocity.y);
        just_jumped = false;
    }
    
    if(behavior.current_state.get()->params.complete) {
        if(grounded) {
           
            behavior.reset();
            
        } else {
            behavior.air(physics.velocity.y);
        }
    }
    
    if((move_left || move_right) && behavior.current_state->params.behavior_id == "idle") {
        if(grounded) {
            behavior.run();
        } else {
            behavior.air(physics.velocity.y);
        }
    }
    
    if(physics.velocity.y > behavior.suspension_threshold && !freefalling) {
        entered_freefall = true;
    }
    if(entered_freefall && !freefalling && !is_wall_sliding) {
        behavior.fall();
        freefalling = true;
        entered_freefall = false;
    }
    
    if(behavior.current_state->params.behavior_id == "suspended") {
        if(grounded) {
            behavior.reset();
        }
    }
    
    
    if(just_landed) {
        behavior.land();
        soundboard_flags.land = true;
        freefalling = false;
    }
    
    behavior::trigger = false;
    
    
    
    if(wall_slide_trigger) { is_wall_sliding = true; }
    if(weapon_fired) { start_cooldown = true; }
    
    stopping = false;
    just_landed = false;
    left_released = false;
    right_released = false;
    wall_slide_trigger = false;
    release_wallslide = false;
    if(!loadout.get_equipped_weapon().attributes.automatic) {
        weapon_fired = false;
    }
    
    if(start_cooldown) {
        loadout.get_equipped_weapon().current_cooldown--;
        if(loadout.get_equipped_weapon().current_cooldown < 0) {
            loadout.get_equipped_weapon().current_cooldown = loadout.get_equipped_weapon().attributes.cooldown_time;
            start_cooldown = false;
        }
    }
    
    if(grounded || (!has_left_collision && !has_right_collision) || abs(physics.velocity.x) > 0.001f) {
        is_wall_sliding = false;
    }
    update_direction();
    update_weapon_direction();
    behavior.flip_left();
    
}

void Player::set_position(sf::Vector2<float> new_pos) {
    physics.position = new_pos;
    sync_components();
}

void Player::update_direction() {
    behavior.facing = last_dir;
    if(behavior.facing_right()) {
        behavior.facing = behavior::DIR::RIGHT;
        if(look_up) {
            behavior.facing = behavior::DIR::UP_RIGHT;
        }
        if(look_down) {
            behavior.facing = behavior::DIR::DOWN_RIGHT;
        }
    }
    if(behavior.facing_left()) {
        behavior.facing = behavior::DIR::LEFT;
        if(look_up) {
            behavior.facing = behavior::DIR::UP_LEFT;
        }
        if(look_down) {
            behavior.facing = behavior::DIR::DOWN_LEFT;
        }
    }
    if(!move_left && !move_right && look_up) {
        if(behavior.facing_strictly_left()) {
            behavior.facing = behavior::DIR::UP_LEFT;
        } else {
            behavior.facing = behavior::DIR::UP_RIGHT;
        }
    }
    if(!move_left && !move_right && look_down) {
        if(behavior.facing_strictly_left()) {
            behavior.facing = behavior::DIR::DOWN_LEFT;
        } else {
            behavior.facing = behavior::DIR::DOWN_RIGHT;
        }
    }
}

void Player::update_weapon_direction() {
    switch(behavior.facing_lr) {
        case behavior::DIR_LR::LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::LEFT;
            physics.dir = components::DIRECTION::LEFT;
            break;
        case behavior::DIR_LR::RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::RIGHT;
            physics.dir = components::DIRECTION::RIGHT;
            break;
    }
    switch(behavior.facing) {
        case behavior::DIR::NEUTRAL:
            break;
        case behavior::DIR::LEFT:
            break;
        case behavior::DIR::RIGHT:
            break;
        case behavior::DIR::UP:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::UP_LEFT;
            physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::DOWN:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_LEFT;
            physics.dir = components::DIRECTION::DOWN;
            break;
        case behavior::DIR::UP_RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::UP_RIGHT;
            physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::UP_LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::UP_LEFT;
            physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::DOWN_RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_RIGHT;
            physics.dir = components::DIRECTION::DOWN;
            break;
        case behavior::DIR::DOWN_LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_LEFT;
            physics.dir = components::DIRECTION::DOWN;
            break;
    }
    loadout.get_equipped_weapon().set_orientation();
    if(behavior.facing_right()) {
        hand_position = {28, 36};
    } else {
        hand_position = {20, 36};
    }
}

void Player::handle_map_collision(const Shape &cell, bool is_ramp) {
    if(left_detector.SAT(cell) && physics.velocity.x < 0.01f && !is_ramp) {
        has_left_collision = true;
        physics.acceleration.x = 0.0f;
        physics.velocity.x = 0.0f;
        left_aabb_counter++;
        if(!grounded && physics.velocity.y > stats.WALL_SLIDE_THRESHOLD && move_left && !is_wall_sliding) {
            wall_slide_trigger = true;
        }
    }
    if(right_detector.SAT(cell) && physics.velocity.x > -0.01f && !is_ramp) {
        has_right_collision = true;
        physics.acceleration.x = 0.0f;
        physics.velocity.x = 0.0f;
        right_aabb_counter++;
        if(!grounded && physics.velocity.y > stats.WALL_SLIDE_THRESHOLD && move_right && !is_wall_sliding) {
            wall_slide_trigger = true;
        }
    }
    
    if(wall_slide_trigger && !wall_slide_detector.SAT(cell)) { release_wallslide = true; }
    
    if(predictive_hurtbox.SAT(cell)) {
        is_any_colllision = true;
        //set mtv
        physics.mtv = predictive_hurtbox.testCollisionGetMTV(predictive_hurtbox, cell);
        
        
        if(physics.velocity.y > 3.0f) {
            physics.mtv.x = 0.0f;
        }
        //here, we can do MTV again with the player's predicted position based on velocity
        if(physics.velocity.y > -0.01f) {
            if(physics.velocity.y > stats.LANDED_THRESHOLD) {
                just_landed = true;
            }
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
//            just_landed = true;
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

sf::Vector2<float> Player::get_fire_point() {
    if(behavior.facing_strictly_left()) {
        return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(-loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f};
    } else if(behavior.facing_strictly_right()) {
        return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f};;
    } else if(behavior.facing_up()) {
        return apparent_position + sf::Vector2<float>{PLAYER_WIDTH/2, 0.0f};
    } else {
        return apparent_position + sf::Vector2<float>{PLAYER_WIDTH/2, PLAYER_HEIGHT};
    }
}

void Player::play_sounds() {
    if(soundboard_flags.jump) { svc::assetLocator.get().jump.play(); }
    if(soundboard_flags.step) {
        if(!(svc::assetLocator.get().step.getStatus() == sf::Sound::Status::Playing) && !(svc::assetLocator.get().landed.getStatus() == sf::Sound::Status::Playing)) {
            svc::assetLocator.get().step.play();
        }
    }
    if(soundboard_flags.land) { svc::assetLocator.get().landed.play(); }
    if(soundboard_flags.weapon_swap) { svc::assetLocator.get().arms_switch.play(); }
    soundboard_flags = SoundboardFlags{false, false, false, false};
}

std::string Player::print_direction(bool lr) {
    if(lr) {
        switch(behavior.facing_lr) {
            case behavior::DIR_LR::LEFT:
                return "LEFT";
                break;
            case behavior::DIR_LR::RIGHT:
                return "RIGHT";
                break;
        }
    }
    switch(behavior.facing) {
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
