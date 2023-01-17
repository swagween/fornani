//
//  Particle.hpp
//  Gems
//
//  Created by Alex Frasca on 01/05/2023.
//
#pragma once

#include "../components/PhysicsComponent.hpp"
#include <vector>

namespace gen {

using Time = std::chrono::duration<float>;
const int default_lifespan = 50;

class Particle {
public:
    
    Particle() = default;
    void update(Time dt, float initial_force) {
        int randx = physics.random_range(-15, 15);
        int randy = physics.random_range(3, 14);
        physics.apply_force({(float)(randx)*initial_force, -1*initial_force*randy});
        physics.update();
        --lifespan;
    }
    components::PhysicsComponent physics{};
    float lifespan = physics.random_range(default_lifespan, 100);
    
};

} // end namespace



/* Particle_hpp */
