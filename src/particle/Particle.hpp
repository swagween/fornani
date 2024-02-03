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
const float default_dim = 2.f;

class Particle {
public:
    
    Particle() = default;
    Particle(components::PhysicsComponent p, float f, float v, float a, sf::Vector2<float> fric, float sz = 3.0f);
    void update(float initial_force, float grav, float grav_variance);
    components::PhysicsComponent physics{};
    float lifespan{};
    float init_force{};
    float force_variance{};
    float angle_range{};
    float size{};
    shape::Shape bounding_box{};
};

} // end namespace



/* Particle_hpp */
