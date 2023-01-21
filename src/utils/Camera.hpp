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

namespace {

const float CAM_FRICTION = 0.9f;
const float CAM_MASS = 1.0f;
const float CAM_GRAV = 0.003f;

const int CX_OFFSET = 60;
const int CY_OFFSET = 60;


class Camera {
public:
    
    Camera() {
        physics = components::PhysicsComponent(CAM_FRICTION, CAM_MASS);
        bounding_box = sf::Rect<float>({0.0f, 0.0f}, {960*2, 540*2});
    }
    
    void move(sf::Vector2<float> new_position) {
        physics.apply_force(physics.position - new_position);
        update();
    }
    
    void update() {
        physics.update();
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
    
    sf::Rect<float> bounding_box{};
    components::PhysicsComponent physics{};
    
};

} // end namespace

/* Camera_hpp */
