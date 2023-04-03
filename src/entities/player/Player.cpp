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
    
    collider = shape::Collider(sf::Vector2<float>{ PLAYER_WIDTH, PLAYER_HEIGHT }, sf::Vector2<float>{ PLAYER_START_X, PLAYER_START_Y });
    collider.physics = components::PhysicsComponent({ stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC }, stats.PLAYER_MASS);
    anchor_point = { collider.physics.position.x + PLAYER_WIDTH/2, collider.physics.position.y + PLAYER_HEIGHT/2};
    behavior.current_state = behavior::Behavior(behavior::idle);
    behavior.facing_lr = behavior::DIR_LR::RIGHT;
    
    weapons_hotbar = {
        arms::WEAPON_TYPE::BRYNS_GUN,
        arms::WEAPON_TYPE::PLASMER,
        arms::WEAPON_TYPE::CLOVER
    };
    loadout.equipped_weapon = weapons_hotbar.at(0);
    
    //sprites
    assign_texture(svc::assetLocator.get().t_nani);
    sprite.setTexture(svc::assetLocator.get().t_nani_red);
    
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
                    behavior.air(collider.physics.velocity.y);
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
                    behavior.air(collider.physics.velocity.y);
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
            collider.has_left_collision = false;
            if(!collider.has_right_collision) {
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
                    behavior.air(collider.physics.velocity.y);
                }
            }
            if(move_right) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }
        }
        if (event.key.code == sf::Keyboard::Right) {
            move_right = false;
            collider.has_right_collision = false;
            if(!collider.has_left_collision) {
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
                    behavior.air(collider.physics.velocity.y);
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
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            is_jump_pressed = false;
            jump_hold = false;
            if(!grounded) { jump_request = -1; }
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
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space) {
            inspecting = true;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Space) {
            inspecting = false;
        }
    }
}

void Player::update(Time dt) {
//    if(behavior::trigger) { behavior.reset(); }
    update_animation();
    //check if player requested jump
    if(grounded && jump_request > -1) {
        if(!behavior.restricted()) {
            jump_height_counter = 0;
        }
        jump_hold = true;
        if(just_jumped) {
            behavior.jump();
        }
    }
    if(!is_jump_pressed) { jump_hold = false; }
    
    //check keystate
    if(!behavior.restricted()) {
        if(move_left && !collider.has_left_collision) {
            if(grounded) {
                collider.physics.acceleration.x = -stats.X_ACC;
            } else {
                collider.physics.acceleration.x = -stats.X_ACC_AIR/stats.AIR_MULTIPLIER;
            }
        }
        if(move_right && !collider.has_right_collision) {
            if(grounded) {
                collider.physics.acceleration.x = stats.X_ACC;
            } else {
                collider.physics.acceleration.x = stats.X_ACC_AIR/stats.AIR_MULTIPLIER;
            }
        }

        if (behavior.current_state.get_frame() == 44 || behavior.current_state.get_frame() == 46) {
            soundboard_flags.step = true;
        }
    }
    
    //zero the player's horizontal acceleration if movement was not requested
    if((!move_left && !move_right)) {
        collider.physics.acceleration.x = 0.0f;
    }
    
    //gravity and stats corrections
    if(!grounded && collider.physics.velocity.y < stats.TERMINAL_VELOCITY) {
        collider.physics.gravity = stats.PLAYER_GRAV;
    } else {
        collider.physics.gravity = 0.0f;
    }
    
    //weapon physics
    if(weapon_fired) {
        if(behavior.facing == behavior::DIR::LEFT) {
            if(!collider.has_right_collision) {
                collider.physics.apply_force({loadout.get_equipped_weapon().attributes.recoil, -loadout.get_equipped_weapon().attributes.recoil/4});
            }
        }
        if(behavior.facing == behavior::DIR::RIGHT) {
            if(!collider.has_left_collision) {
                collider.physics.apply_force({-loadout.get_equipped_weapon().attributes.recoil, -loadout.get_equipped_weapon().attributes.recoil/4});
            }
        }
    }
    
    if(jump_request > -1) {
        if(!behavior.restricted()) {
            jump_request--;
            //still jump for quick presses
            if(grounded) {
                collider.physics.acceleration.y = -stats.JUMP_MAX / 1.3f;
                ++jump_height_counter;
                can_jump = false;
                jump_trigger = false;
                behavior.rise();
                if(!jump_trigger) { soundboard_flags.jump = true; }
            }
        }
    }

    
    if(move_left && move_right) {
        collider.physics.acceleration.x = 0.0f;
    }
    
    collider.physics.update_euler();
    
    
    collider.sync_components();
    
    //for parameter tweaking, remove later
    if(grounded) {
        collider.physics.friction = {stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC};
    } else {
        collider.physics.friction = {stats.PLAYER_HORIZ_AIR_FRIC, stats.PLAYER_VERT_AIR_FRIC };
    }
    if(!collider.is_colliding_with_level) {
        collider.physics.mtv = {0.0f, 0.0f};
    }
    collider.just_collided = false;
    
    update_behavior();
    apparent_position.x = collider.physics.position.x + PLAYER_WIDTH/2;
    apparent_position.y = collider.physics.position.y;
    play_sounds();
}

void Player::render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
    
    sf::Vector2<float> player_pos = apparent_position - campos;
    
    //get UV coords
    int u = (int)(behavior.get_frame() / NANI_SPRITESHEET_HEIGHT) * NANI_SPRITE_WIDTH;
    int v = (int)(behavior.get_frame() % NANI_SPRITESHEET_HEIGHT) * NANI_SPRITE_WIDTH;
    sprite.setTextureRect(sf::IntRect({u, v}, {NANI_SPRITE_WIDTH, NANI_SPRITE_WIDTH}));
    sprite.setOrigin(NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2);
    sprite.setPosition(player_pos.x, player_pos.y);
    
    //flip the sprite based on the player's direction
    sf::Vector2<float> right_scale = {1.0f, 1.0f};
    sf::Vector2<float> left_scale = {-1.0f, 1.0f};
    if(behavior.facing_lr == behavior::DIR_LR::LEFT && sprite.getScale() == right_scale) {
        sprite.scale(-1.0f, 1.0f);
    }
    if(behavior.facing_lr == behavior::DIR_LR::RIGHT && sprite.getScale() == left_scale) {
        sprite.scale(-1.0f, 1.0f);
    }
    win.draw(sprite);
    
}

void Player::assign_texture(sf::Texture& tex) {
    sprite.setTexture(tex);
}

void Player::update_animation() {
    behavior.end_loop();
    behavior.current_state.update();
}

void Player::update_behavior() {
    
    
    if(just_jumped && !is_wall_sliding) {
        behavior.air(collider.physics.velocity.y);
        just_jumped = false;
    }
    
    if(behavior.current_state.params.complete) {
        if(grounded) {
           
            behavior.reset();
            
        } else {
            behavior.air(collider.physics.velocity.y);
        }
    }
    
    if((move_left || move_right) && behavior.current_state.params.behavior_id == "idle") {
        if(grounded) {
            behavior.run();
        } else {
            behavior.air(collider.physics.velocity.y);
        }
    }
    
    if(collider.physics.velocity.y > behavior.suspension_threshold && !freefalling) {
        entered_freefall = true;
    }
    if(entered_freefall && !freefalling && !is_wall_sliding) {
        behavior.fall();
        freefalling = true;
        entered_freefall = false;
    }
    
    if(behavior.current_state.params.behavior_id == "suspended") {
        if(grounded) {
            behavior.reset();
        }
    }
    
    
    if(collider.just_landed) {
        behavior.land();
        soundboard_flags.land = true;
        freefalling = false;
    }
    
    if(wall_slide_trigger) { is_wall_sliding = true; }
    if(weapon_fired) { start_cooldown = true; }
    
    stopping = false;
    collider.just_landed = false;
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
    
    if(grounded || (!collider.has_left_collision && !collider.has_right_collision) || abs(collider.physics.velocity.x) > 0.001f) {
        is_wall_sliding = false;
    }
    update_direction();
    update_weapon_direction();
    
}

void Player::set_position(sf::Vector2<float> new_pos) {
    collider.physics.position = new_pos;
    collider.sync_components();
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
    if (behavior.facing_left()) {
        anchor_point = { collider.physics.position.x + collider.bounding_box.shape_w / 2 - ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.shape_h / 2 };
    }
    else if (behavior.facing_right()) {
        anchor_point = { collider.physics.position.x + collider.bounding_box.shape_w / 2 + ANCHOR_BUFFER, collider.physics.position.y + collider.bounding_box.shape_h / 2 };
    }
    else {
        anchor_point = { collider.physics.position.x + collider.bounding_box.shape_w / 2, collider.physics.position.y + collider.bounding_box.shape_h / 2 };
    }
}

void Player::update_weapon_direction() {
    switch(behavior.facing_lr) {
        case behavior::DIR_LR::LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::LEFT;
            collider.physics.dir = components::DIRECTION::LEFT;
            break;
        case behavior::DIR_LR::RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::RIGHT;
            collider.physics.dir = components::DIRECTION::RIGHT;
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
            collider.physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::DOWN:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_LEFT;
            collider.physics.dir = components::DIRECTION::DOWN;
            break;
        case behavior::DIR::UP_RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::UP_RIGHT;
            collider.physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::UP_LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::UP_LEFT;
            collider.physics.dir = components::DIRECTION::UP;
            break;
        case behavior::DIR::DOWN_RIGHT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_RIGHT;
            collider.physics.dir = components::DIRECTION::DOWN;
            break;
        case behavior::DIR::DOWN_LEFT:
            loadout.get_equipped_weapon().sprite_orientation = arms::WEAPON_DIR::DOWN_LEFT;
            collider.physics.dir = components::DIRECTION::DOWN;
            break;
    }
    loadout.get_equipped_weapon().set_orientation();
    if(behavior.facing_right()) {
        hand_position = {28, 36};
    } else {
        hand_position = {20, 36};
    }
}

sf::Vector2<float> Player::get_fire_point() {
    if(behavior.facing_strictly_left()) {
        return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(-loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f} - sf::Vector2<float>{NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2};
    } else if(behavior.facing_strictly_right()) {
        return apparent_position + hand_position + sf::Vector2<float>{static_cast<float>(loadout.get_equipped_weapon().sprite_dimensions.x), 0.0f} - sf::Vector2<float>{NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2};
    } else if(behavior.facing_up()) {
        return apparent_position + sf::Vector2<float>{PLAYER_WIDTH/2, 0.0f} - sf::Vector2<float>{NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2};
    } else {
        return apparent_position + sf::Vector2<float>{PLAYER_WIDTH/2, PLAYER_HEIGHT} - sf::Vector2<float>{NANI_SPRITE_WIDTH/2, NANI_SPRITE_WIDTH/2};
    }
}

void Player::play_sounds() {

    if (soundboard_flags.land) { svc::assetLocator.get().landed.play(); }
    if(soundboard_flags.jump) { svc::assetLocator.get().jump.play(); }
    if(soundboard_flags.step) {
        if(!(svc::assetLocator.get().step.getStatus() == sf::Sound::Status::Playing)) {
            util::Random r{};
            float randp = r.random_range_float(0.0f, 0.1f);
            svc::assetLocator.get().step.setPitch(1.0f + randp);

            svc::assetLocator.get().step.setVolume(60);
            svc::assetLocator.get().step.play();
        }
    }
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
