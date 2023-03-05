//
//  Camera.hpp
//
//
//  Created by Alex Frasca on 12/26/22.
//
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "../components/PhysicsComponent.hpp"
#include "../utils/Random.hpp"

namespace {

const float CAM_FRICTION = 0.9f;
const float CAM_MASS = 1.0f;
const float CAM_GRAV = 0.003f;

const int CX_OFFSET = 60;
const int CY_OFFSET = 60;

const int SHAKE_FACTOR = 8;
const int SHAKE_VOLATILITY = 12;
const int SHAKE_DURATION = 100;

const sf::Vector2<uint32_t> aspect_ratio { 3840, 2160 };
const sf::Vector2<uint32_t> screen_dimensions { aspect_ratio.x / 4, aspect_ratio.y / 4 };


class Camera {
public:
    
    Camera() {
        physics = components::PhysicsComponent({CAM_FRICTION, CAM_FRICTION}, CAM_MASS);
        bounding_box = sf::Rect<float>({0.0f, 0.0f}, {(float)screen_dimensions.x, (float)screen_dimensions.y});
    }
    
    void move(sf::Vector2<float> new_position) {
        physics.apply_force(physics.position - new_position);
        update();
    }
    
    void update() {
        physics.update_dampen();
        bounding_box.left = physics.position.x;
        bounding_box.top = physics.position.y;
        if(bounding_box.top < 0) {
            bounding_box.top = 0.0f;
            physics.position.y = 0.0f;
        }
        if(bounding_box.left < 0) {
            bounding_box.left = 0.0f;
            physics.position.x = 0.0f;
        }
        if(shaking) {
            if(shake_counter % SHAKE_VOLATILITY == 0) {
                shake();
            }
            shake_counter++;
            if(shake_counter > SHAKE_DURATION) {
                shake_counter = 0;
                shaking = false;
            }
        }
    }
    
    void restrict_movement(sf::Vector2<float>& bounds) {
        if(bounding_box.top + bounding_box.height > bounds.y) {
            bounding_box.top = bounds.y - bounding_box.height;
            physics.position.y = bounds.y - bounding_box.height;
        }
        if(bounding_box.left + bounding_box.width > bounds.x) {
            bounding_box.left = bounds.x - bounding_box.width;
            physics.position.x = bounds.x - bounding_box.width;
        }
    }
    
    void set_position(sf::Vector2<float> new_pos) {
        physics.position = new_pos;
    }
    
    void center(sf::Vector2<float> new_position) {
        float gx = physics.position.x;
        float gy = physics.position.y;
        float mx = new_position.x - bounding_box.width / 2;
        float my = new_position.y - bounding_box.height / 2;
        
        float force_x = mx - gx;
        float force_y = my - gy;
        float mag = sqrt((force_x*force_x) + (force_y*force_y));
        float str = CAM_GRAV / mag*mag;
        force_x *= str;
        force_y *= str;
        physics.apply_force({force_x, force_y});
        update();
    }
    
    void begin_shake() {
        shaking = true;
    }
    
    void shake() {
        float nudge_x = rand.random_range(-SHAKE_FACTOR, SHAKE_FACTOR);
        float nudge_y = rand.random_range(-SHAKE_FACTOR, SHAKE_FACTOR);
        physics.velocity.x = nudge_x*0.1;
        physics.velocity.y = nudge_y*0.1;
    }
    
    sf::Rect<float> bounding_box{};
    components::PhysicsComponent physics{};
    
    int shake_counter{};
    bool shaking = false;
    
    util::Random rand{};
    
};

} // end namespace

/* Camera_hpp */
