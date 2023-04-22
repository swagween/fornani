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
    
    /*weapons_hotbar = {
        arms::WEAPON_TYPE::BRYNS_GUN,
        arms::WEAPON_TYPE::PLASMER,
        arms::WEAPON_TYPE::CLOVER
    };
    loadout.equipped_weapon = weapons_hotbar.at(0);*/
    
    //sprites
    assign_texture(svc::assetLocator.get().t_nani);
    sprite.setTexture(svc::assetLocator.get().t_nani_red);
    
}

void Player::handle_events(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Left) {
            flags.movement.set(Movement::move_left);
            if(grounded()) {
                if(behavior.facing == behavior::DIR::RIGHT) {
                    behavior.turn();
                    behavior.facing = behavior::DIR::LEFT;
                    flags.movement.reset(Movement::is_wall_sliding);
                } else {
                    behavior.run();
                }
            } else if (!flags.input.test(Input::restricted)) {
                if(!flags.movement.test(Movement::is_wall_sliding)) {
                    behavior.air(collider.physics.velocity.y);
                }
            }
            behavior.facing = behavior::DIR::LEFT;
            behavior.facing_lr = behavior::DIR_LR::LEFT;
        }
        if (event.key.code == sf::Keyboard::Right) {
            flags.movement.set(Movement::move_right);
            if(grounded()) {
                if(behavior.facing == behavior::DIR::LEFT) {
                    behavior.turn();
                    flags.movement.reset(Movement::is_wall_sliding);
                } else {
                    behavior.run();
                }
            } else if (!flags.input.test(Input::restricted)) {
                if(!flags.movement.test(Movement::is_wall_sliding)) {
                    behavior.air(collider.physics.velocity.y);
                }
            }
            behavior.facing = behavior::DIR::RIGHT;
            behavior.facing_lr = behavior::DIR_LR::RIGHT;
        }
        if (event.key.code == sf::Keyboard::Up) {
            flags.movement.set(Movement::look_up);
        }
        if (event.key.code == sf::Keyboard::Down) {
            flags.movement.set(Movement::look_down);
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (!flags.movement.test(Movement::autonomous_walk)) {
            if (event.key.code == sf::Keyboard::Left) {
                flags.movement.reset(Movement::move_left);
                collider.has_left_collision = false;
                if (!collider.has_right_collision) {
                    flags.movement.reset(Movement::is_wall_sliding);
                }
                flags.movement.set(Movement::stopping);
                last_dir = behavior::DIR::LEFT;
                flags.movement.set(Movement::left_released);
                if (grounded()) {
                    if (!flags.movement.test(Movement::move_right) && !behavior.restricted()) {
                        behavior.reset();
                    }
                }
                else {
                    if (!behavior.restricted() && !flags.movement.test(Movement::is_wall_sliding)) {
                        behavior.air(collider.physics.velocity.y);
                    }
                }
                if (flags.movement.test(Movement::move_right)) { behavior.facing_lr = behavior::DIR_LR::RIGHT; }
            }
            if (event.key.code == sf::Keyboard::Right) {
                flags.movement.reset(Movement::move_right);
                collider.has_right_collision = false;
                if (!collider.has_left_collision) {
                    flags.movement.reset(Movement::is_wall_sliding);
                }
                flags.movement.set(Movement::stopping);
                last_dir = behavior::DIR::RIGHT;
                flags.movement.set(Movement::right_released);
                if (grounded()) {
                    if (!behavior.restricted()) {
                        behavior.reset();
                    }
                }
                else {
                    if (!behavior.restricted() && !flags.movement.test(Movement::is_wall_sliding)) {
                        behavior.air(collider.physics.velocity.y);
                    }
                }
                if (flags.movement.test(Movement::move_left)) { behavior.facing_lr = behavior::DIR_LR::LEFT; }
            }
        }
        if (event.key.code == sf::Keyboard::Up) {
            flags.movement.reset(Movement::look_up);
        }
        if (event.key.code == sf::Keyboard::Down) {
            flags.movement.reset(Movement::look_down);
        }
        if (event.key.code == sf::Keyboard::X) {
            weapon_fired = false;
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Z && !flags.input.test(Input::restricted)) {
            flags.jump.set(Jump::is_pressed);
            jump_request = JUMP_BUFFER_TIME;
            flags.jump.set(Jump::just_jumped);
            flags.jump.set(Jump::trigger);
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Z) {
            if (flags.jump.test(Jump::just_jumped) || flags.jump.test(Jump::hold) || flags.jump.test(Jump::jumping) || jump_request > -1) {
                flags.jump.set(Jump::is_released);
            }
            flags.jump.reset(Jump::is_pressed);
            flags.jump.reset(Jump::hold);
            if(!grounded()) { jump_request = -1; }
            if(!behavior.restricted()) {
                flags.jump.set(Jump::can_jump);
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
                flags.sounds.set(Soundboard::weapon_swap);
            }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::S) {
            if(!weapons_hotbar.empty()) {
                current_weapon++;
                if(current_weapon > weapons_hotbar.size() - 1) { current_weapon = 0; }
                loadout.equipped_weapon = weapons_hotbar.at(current_weapon);
                flags.sounds.set(Soundboard::weapon_swap);
            }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Down) {
            if (grounded()) {
                flags.input.set(Input::inspecting_trigger);
            }
        }
    }
}

void Player::update(Time dt) {

    if (!flags.input.test(Input::no_anim)) {
        update_animation();
        update_sprite();
    }

    if (!flags.input.test(Input::restricted)) {

        if (moving() || flags.movement.test(Movement::look_up) || jump_request > -1) {
            flags.input.reset(Input::inspecting);
            flags.input.reset(Input::inspecting_trigger);
        }

        if (flags.input.test(Input::inspecting_trigger) && behavior.current_state.params.behavior_id == "inspecting" && behavior.current_state.params.done) {
            flags.input.set(Input::inspecting);
        }

        //check if player requested jump
        if (grounded() && jump_request > -1) {
            flags.jump.set(Jump::hold);
            if (flags.jump.test(Jump::just_jumped)) {
                behavior.jump();
            }
        }

        //preset flags
        if (grounded()) { flags.jump.reset(Jump::jumping); } //do this before jumping, otherwise it will trigger on the same frame

        //jump!
        if (jump_request > -1) {
            if (!behavior.restricted()) { //once jumpsquat is over
                jump_request--;
                if (grounded()) {
                    collider.physics.acceleration.y = -stats.JUMP_MAX / 1.3f;
                    flags.jump.set(Jump::jumping);
                    flags.jump.reset(Jump::can_jump);
                    flags.jump.reset(Jump::trigger);
                    behavior.rise();
                    if (!flags.jump.test(Jump::trigger)) {
                        flags.sounds.set(Soundboard::jump);
                    }
                }
            }
        }

        //reset jump flags
        if (!flags.jump.test(Jump::is_pressed)) { flags.jump.reset(Jump::hold); }
        if (grounded() && jump_request == -1) { flags.jump.reset(Jump::is_released); }
        if (flags.jump.test(Jump::is_released) && flags.jump.test(Jump::jumping)) { collider.physics.acceleration.y *= stats.JUMP_RELEASE_MULTIPLIER; flags.jump.reset(Jump::jumping); } //player lets go
        if (flags.jump.test(Jump::is_released) && !grounded()) { flags.jump.reset(Jump::is_released); }

        //check keystate
        if (!behavior.restricted()) {
            walk();
        }

        //zero the player's horizontal acceleration if movement was not requested
        if (!moving()) {
            collider.physics.acceleration.x = 0.0f;
            if (abs(collider.physics.velocity.x) > 0.4f && grounded()) { flags.movement.set(Movement::just_stopped); }
        }

        //gravity and stats corrections
        if (!grounded() && collider.physics.velocity.y < stats.TERMINAL_VELOCITY) {
            collider.physics.gravity = stats.PLAYER_GRAV;
        }
        else {
            collider.physics.gravity = 0.0f;
        }

        //weapon physics
        if (weapon_fired && !weapons_hotbar.empty()) {
            if (behavior.facing_strictly_right()) {
                if (!collider.has_right_collision) {
                    collider.physics.acceleration.x += -loadout.get_equipped_weapon().attributes.recoil;
                }
            }
            if (behavior.facing_strictly_left()) {
                if (!collider.has_left_collision) {
                    collider.physics.acceleration.x += loadout.get_equipped_weapon().attributes.recoil;
                }
            }
            if (behavior.facing_down()) {
                collider.physics.acceleration.y += -loadout.get_equipped_weapon().attributes.recoil / 8;
            }
            if (behavior.facing_up()) {
                collider.physics.acceleration.y += loadout.get_equipped_weapon().attributes.recoil;
            }
        }

        if (flags.movement.test(Movement::move_left) && flags.movement.test(Movement::move_right)) {
            collider.physics.acceleration.x = 0.0f;
        }
    }
    
    collider.physics.update_euler();
    
    collider.sync_components();
    
    //for parameter tweaking, remove later
    collider.physics.friction = grounded() ? sf::Vector2<float>{stats.PLAYER_GROUND_FRIC, stats.PLAYER_GROUND_FRIC} : sf::Vector2<float>{stats.PLAYER_HORIZ_AIR_FRIC, stats.PLAYER_VERT_AIR_FRIC };
    if(!collider.is_colliding_with_level) { collider.physics.mtv = {0.0f, 0.0f}; }
    collider.just_collided = false;

    //hurt
    if (is_invincible()) { collider.spike_trigger = false; flash_sprite(); }
    else { sprite.setColor(sf::Color::White); }
    just_hurt = collider.spike_trigger;
    if (collider.spike_trigger && !is_invincible()) { collider.physics.acceleration.y = -stats.HURT_ACC; collider.spike_trigger = false; make_invincible(); flags.sounds.set(Soundboard::hurt); --player_stats.health; }
    update_invincibility();
    if (player_stats.health <= 0) { kill(); }

    play_sounds();
    update_behavior();
    apparent_position.x = collider.physics.position.x + PLAYER_WIDTH/2;
    apparent_position.y = collider.physics.position.y;
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
    if (flags.state.test(State::alive)) {
        win.draw(sprite);
    }
    
}

void Player::assign_texture(sf::Texture& tex) {
    sprite.setTexture(tex);
}

void Player::update_animation() {
    behavior.end_loop();
    behavior.current_state.update();
}

void Player::update_sprite() {
    if(weapons_hotbar.empty()) {
        sprite.setTexture(svc::assetLocator.get().t_nani_unarmed);
    }
}

void Player::flash_sprite() {
    if ((counters.invincibility / 10) % 2 == 0) { sprite.setColor(flcolor::red); }
    else { sprite.setColor(flcolor::blue); }
}

void Player::update_behavior() {
    
    
    if(flags.jump.test(Jump::just_jumped) && !flags.movement.test(Movement::is_wall_sliding)) {
        behavior.air(collider.physics.velocity.y);
        flags.jump.reset(Jump::just_jumped);
    }
    
    if(behavior.current_state.params.complete) {
        if(grounded()) {
            behavior.reset();
        } else {
            behavior.air(collider.physics.velocity.y);
        }
    }

    if (flags.movement.test(Movement::autonomous_walk)) {
        behavior.run();
    }
    
    if(moving() && behavior.current_state.params.behavior_id == "idle") {
        if(grounded()) {
            behavior.run();
        } else {
            behavior.air(collider.physics.velocity.y);
        }
    }

    if(flags.movement.test(Movement::just_stopped)) {
        behavior.stop();
    }

    if(flags.input.test(Input::inspecting_trigger) && !(behavior.current_state.params.behavior_id == "inspecting")) {
        behavior.inspect();
    }
    
    if(collider.physics.velocity.y > behavior.suspension_threshold && !flags.movement.test(Movement::freefalling)) {
        flags.movement.set(Movement::entered_freefall);
    }
    if(flags.movement.test(Movement::entered_freefall) && !flags.movement.test(Movement::freefalling) && !flags.movement.test(Movement::is_wall_sliding)) {
        behavior.fall();
        flags.movement.set(Movement::freefalling);
        flags.movement.reset(Movement::entered_freefall);
    }
    
    if(behavior.current_state.params.behavior_id == "suspended") {
        if(grounded()) {
            behavior.reset();
        }
    }
    
    
    if(collider.just_landed && jump_request == -1) {
        behavior.land();
        flags.sounds.set(Soundboard::land);
        flags.movement.reset(Movement::freefalling);
    }

    if(just_hurt) {
        behavior.hurt();
    }
    
    if (flags.movement.test(Movement::wall_slide_trigger)) { flags.movement.set(Movement::is_wall_sliding); }
    if(weapon_fired) { start_cooldown = true; }
    

    flags.movement.reset(Movement::stopping);
    flags.movement.reset(Movement::just_stopped);
    collider.just_landed = false;
    flags.movement.reset(Movement::left_released);
    flags.movement.reset(Movement::right_released);
    flags.movement.reset(Movement::wall_slide_trigger);
    flags.movement.reset(Movement::release_wallslide);
    if (!weapons_hotbar.empty()) {
        if (!loadout.get_equipped_weapon().attributes.automatic) {
            weapon_fired = false;
        }
        if (start_cooldown) {
            loadout.get_equipped_weapon().current_cooldown--;
            if (loadout.get_equipped_weapon().current_cooldown < 0) {
                loadout.get_equipped_weapon().current_cooldown = loadout.get_equipped_weapon().attributes.cooldown_time;
                start_cooldown = false;
            }
        }
    }
    
    if(grounded() || (!collider.has_left_collision && !collider.has_right_collision) || abs(collider.physics.velocity.x) > 0.001f) {
        flags.movement.reset(Movement::is_wall_sliding);
    }
    update_direction();
    if (!weapons_hotbar.empty()) {
        update_weapon_direction();
    }
    
}

void Player::set_position(sf::Vector2<float> new_pos) {
    collider.physics.position = new_pos;
    collider.sync_components();
}

void Player::update_direction() {
    behavior.facing = last_dir;
    if(behavior.facing_right()) {
        behavior.facing = behavior::DIR::RIGHT;
        if(flags.movement.test(Movement::look_up)) {
            behavior.facing = behavior::DIR::UP_RIGHT;
        }
        if(flags.movement.test(Movement::look_down) && !grounded()) {
            behavior.facing = behavior::DIR::DOWN_RIGHT;
        }
    }
    if(behavior.facing_left()) {
        behavior.facing = behavior::DIR::LEFT;
        if(flags.movement.test(Movement::look_up)) {
            behavior.facing = behavior::DIR::UP_LEFT;
        }
        if(flags.movement.test(Movement::look_down) && !grounded()) {
            behavior.facing = behavior::DIR::DOWN_LEFT;
        }
    }
    if(!moving() && flags.movement.test(Movement::look_up)) {
        if(behavior.facing_strictly_left()) {
            behavior.facing = behavior::DIR::UP_LEFT;
        } else {
            behavior.facing = behavior::DIR::UP_RIGHT;
        }
    }
    if(!flags.movement.test(Movement::move_left) && !flags.movement.test(Movement::move_right) && flags.movement.test(Movement::look_down) && !grounded()) {
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

void Player::walk() {
    if (flags.movement.test(Movement::move_right) && !collider.has_right_collision) {
        collider.physics.acceleration.x = grounded() ? stats.X_ACC : (stats.X_ACC_AIR / stats.AIR_MULTIPLIER);
    }
    if (flags.movement.test(Movement::move_left) && !collider.has_left_collision) {
        collider.physics.acceleration.x = grounded() ? -stats.X_ACC : (-stats.X_ACC_AIR / stats.AIR_MULTIPLIER);
    }
    if (behavior.current_state.get_frame() == 44 || behavior.current_state.get_frame() == 46) {
        if (behavior.current_state.params.frame_trigger) {
            flags.sounds.set(Soundboard::step);
        }
    }
}

void Player::autonomous_walk() {
    collider.physics.acceleration.x = grounded() ? stats.X_ACC : (stats.X_ACC_AIR / stats.AIR_MULTIPLIER);
    if (behavior.facing_lr == behavior::DIR_LR::LEFT) { collider.physics.acceleration.x *= -1.f; }
    flags.movement.set(Movement::autonomous_walk);
    if (behavior.current_state.get_frame() == 44 || behavior.current_state.get_frame() == 46) {
        if (behavior.current_state.params.frame_trigger) {
            flags.sounds.set(Soundboard::step);
        }
    }
}

void Player::restrict_inputs() {

    flags.input.set(Input::restricted);
    flags.movement.reset(Movement::look_down);
    flags.movement.reset(Movement::look_up);
    flags.input.reset(Input::inspecting_trigger);
    weapon_fired = false;

}

void Player::unrestrict_inputs() {
    flags.input.reset(Input::restricted);
    flags.input.reset(Input::no_anim);
    flags.movement.reset(Movement::autonomous_walk);
}

void Player::restrict_animation() {
    flags.input.set(Input::no_anim);
}

void Player::no_move() {
    flags.movement.reset(Movement::move_right);
    flags.movement.reset(Movement::move_left);
}

bool Player::grounded() {
    return flags.movement.test(Movement::grounded);
}

bool Player::moving() {
    return flags.movement.test(Movement::move_left) || flags.movement.test(Movement::move_right);
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

void Player::make_invincible() {
    counters.invincibility = INVINCIBILITY_TIME;
}

void Player::update_invincibility() {
    dt = svc::clockLocator.get().tick_rate;

    auto new_time = Clock::now();
    Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

    if (frame_time.count() > svc::clockLocator.get().frame_limit) {
        frame_time = Time{ svc::clockLocator.get().frame_limit };
    }
    current_time = new_time;
    accumulator += frame_time;

    int integrations = 0;
    if (accumulator >= dt) {

        --counters.invincibility;
        if (counters.invincibility < 0) { counters.invincibility = 0; }

        accumulator = Time::zero();
        ++integrations;
    }
}

bool Player::is_invincible() {
    return counters.invincibility > 0;
}

void Player::kill() {
    flags.state.reset(State::alive);
}

void Player::start_over() {
    player_stats.health = player_stats.max_health;
    flags.state.set(State::alive);
}

void Player::play_sounds() {

    if (flags.sounds.test(Soundboard::land)) { svc::assetLocator.get().landed.play(); }
    if (flags.sounds.test(Soundboard::jump)) { svc::assetLocator.get().jump.play(); }
    if (flags.sounds.test(Soundboard::step)) {
            util::Random r{};
            float randp = r.random_range_float(0.0f, 0.1f);
            svc::assetLocator.get().step.setPitch(1.0f + randp);

            svc::assetLocator.get().step.setVolume(60);
            svc::assetLocator.get().step.play();
    }
    if (flags.sounds.test(Soundboard::weapon_swap)) { svc::assetLocator.get().arms_switch.play(); }
    if (flags.sounds.test(Soundboard::hurt)) { svc::assetLocator.get().hurt.play(); }
    flags.sounds = {};
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
