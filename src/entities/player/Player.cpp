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
        if (event.key.code == sf::Keyboard::Up) {
            move_up = true;
        }
        if (event.key.code == sf::Keyboard::Down) {
            move_down = true;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        if (event.key.code == sf::Keyboard::Up) {
            move_up = false;
        }
        if (event.key.code == sf::Keyboard::Down) {
            move_down = false;
        }
    }
}

void Player::update() {
    
    //check keystate
    if(move_left) {
        physics.apply_force({-X_ACC, 0.0f});
    }
    if(move_right) {
        physics.apply_force({X_ACC, 0.0f});
    }
    if(move_up) {
        physics.apply_force({0.0f, -X_ACC});
    }
    if(move_down) {
        physics.apply_force({0.0f, X_ACC});
    }
    
    sf::operator+=(physics.position, mtv);
    //gravity (off for now)
    if(grav) {
        physics.apply_force({0.0f, PLAYER_GRAV});
    }
    mtv = {0.0f, 0.0f};
    
    physics.update();
    
    //impose physics limitations
    if(physics.velocity.x > PLAYER_MAX_XVEL) {
        physics.velocity.x = PLAYER_MAX_XVEL;
    }
    if(physics.velocity.x < -PLAYER_MAX_XVEL) {
        physics.velocity.x = -PLAYER_MAX_XVEL;
    }
    
    sync_components();
}

void Player::render() {
    
}

void Player::sync_components() {
    hurtbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, PLAYER_HEIGHT);
    jumpbox.update(physics.position.x, physics.position.y, PLAYER_WIDTH, JUMPBOX_HEIGHT);
}

void Player::set_position(sf::Vector2<float> new_pos) {
    physics.position = new_pos;
    sync_components();
}
