//
//  Particle.hpp
//  fornani
//
//  Created by Alex Frasca on 01/05/2023.
//
#pragma once

#include "../components/PhysicsComponent.hpp"
#include "../utils/Shape.hpp"
#include <vector>

namespace vfx {

using Time = std::chrono::duration<float>;
const int default_lifespan = 50;

class Particle {
public:
    
    Particle() = default;
    Particle(components::PhysicsComponent p, float f, sf::Vector2<float> fric) : physics(p), init_force(f) {
        physics.friction = fric;
        util::Random r{};
        float randx{};
        float randy{};
        switch(physics.dir) {
            case components::DIRECTION::LEFT:
                randx = r.random_range_float(-1.0, -0.4);
                randy = r.random_range_float(-1.5, 1.5);
                break;
            case components::DIRECTION::RIGHT:
                randx = r.random_range_float(0.4, 1.0);
                randy = r.random_range_float(-1.5, 1.5);
                break;
            case components::DIRECTION::UP:
                randx = r.random_range_float(-1.5, 1.5);
                randy = r.random_range_float(-1.0, -0.4);
                break;
            case components::DIRECTION::DOWN:
                randx = r.random_range_float(-1.5, 1.5);
                randy = r.random_range_float(0.4, 1.0);
                break;
        }
        physics.apply_force({(float)(randx)*init_force, init_force*randy});
    }
    void update(float initial_force, float grav, float grav_variance) {
        util::Random r{};
        float var = r.random_range_float(-grav_variance, grav_variance);
        physics.acceleration.y = grav + var;
        physics.update_dampen();
        bounding_box.update(physics.position.x, physics.position.y, 2, 2);
        --lifespan;
    }
    components::PhysicsComponent physics{};
    float lifespan = physics.random_range(default_lifespan, 100);
    float init_force{};
    Shape bounding_box{};
    
};

} // end namespace



/* Particle_hpp */
