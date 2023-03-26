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
    Particle(components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric) : physics(p), init_force(f), force_variance(v), angle_range(a) {
        physics.friction = fric;
        float randx{};
        float randy{};
        switch(physics.dir) {
            case components::DIRECTION::LEFT:
                randx = r.random_range_float(init_force - force_variance, init_force + force_variance) * -1;
                randy = r.random_range_float(-angle_range, angle_range);
                break;
            case components::DIRECTION::RIGHT:
                randx = r.random_range_float(init_force - force_variance, init_force + force_variance);
                randy = r.random_range_float(-angle_range, angle_range);
                break;
            case components::DIRECTION::UP:
                randx = r.random_range_float(-angle_range, angle_range);
                randy = r.random_range_float(init_force - force_variance, init_force + force_variance) * -1;
                break;
            case components::DIRECTION::DOWN:
                randx = r.random_range_float(-angle_range, angle_range);
                randy = r.random_range_float(init_force - force_variance, init_force + force_variance);
                break;
        }
        physics.velocity.x = randx*init_force;
        physics.velocity.y = randy*init_force;
//        physics.apply_force({randx*init_force, randy*init_force});
    }
    void update(float initial_force, float grav, float grav_variance) {
        float var = r.random_range_float(-grav_variance, grav_variance);
        physics.acceleration.y = grav + var;
        physics.update_dampen();
        bounding_box.update(physics.position.x, physics.position.y, 2, 2);
        --lifespan;
    }
    components::PhysicsComponent physics{};
    float lifespan = r.random_range(default_lifespan, 100);
    float init_force{};
    float force_variance{};
    float angle_range{};
    Shape bounding_box{};
    util::Random r{};
};

} // end namespace



/* Particle_hpp */
