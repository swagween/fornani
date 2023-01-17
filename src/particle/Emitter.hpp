//
//  Emitter.hpp
//  Gems
//
//  Created by Alex Frasca on 01/05/2023.
//
#pragma once

#include "ObjectPool.hpp"
#include "Particle.hpp"
#include <vector>

namespace gen {

const uint32_t default_size = 64;
using Time = std::chrono::duration<float>;

enum class EmitterType {
    et_infinite,
    et_burst,
    et_fadeout,
    et_fadein
};

struct ElementBehavior {
    float rate{}; //expulsion rate
    float rate_variance{}; //variance in expulsion rate
    float expulsion_force{};
    float expulsion_variance{};
    float cone{}; //angle in radians of element dispersal
};

class Emitter {
    
public:
    
    Emitter() = default;
    Emitter(ElementBehavior b) : behavior(b) { }
    
    void update(Time dt) { //this will tick every element and the generator itself
        elapsed += dt;
        physics.update();
        if(elapsed > static_cast<Time>(behavior.rate)) { //make a particle at a certain rate
            elements.push(Particle());
            particles.push_back(Particle());
            particles.back().lifespan = lifespan;
            particles.back().physics = physics; //initialize the particles with emitter's physics
            elapsed = Time::zero();
        }
        for(int i = static_cast<int>(particles.size()) - 1; i >= 0; --i) {
            particles.at(i).update(dt, behavior.expulsion_force);
            if(particles.at(i).lifespan < 0) {
                particles.erase(particles.begin() + i); //this is bad. I want to delete the element at that spot
            }
        }
    }
    
    void set_size(uint32_t sz);
    void set_type(EmitterType t);
    
    void set_position(float x, float y) {
        physics.position.x = x;
        physics.position.y = y;
        for(auto& particle : particles) {
            particle.physics.position = {x, y};
        }
    }
    void set_velocity(float x, float y) {
        physics.velocity.x = x;
        physics.velocity.y = y;
    }
    
    void apply_force(sf::Vector2<float> force) {
        physics.apply_force(force);
        for(auto& particle : particles) {
            particle.physics.apply_force(force);
        }
    }
    void apply_force_at_angle(float force, float angle) {
        physics.apply_force_at_angle(force, angle);
        for(auto& particle : particles) {
            particle.physics.apply_force_at_angle(force, angle);
        }
    }
    
    components::PhysicsComponent& get_physics() {
        return physics;
    }
    
    void set_rate(float r) { behavior.rate = r; }
    void set_expulsion_force(float f) { behavior.expulsion_force = f; }
    void set_friction(float f) { physics.friction = f; }
    void set_lifespan(int l) { lifespan = l; }
    
    std::vector<Particle>& get_particles() {
        return particles;
    }
    
    int num_particles () {
        return (int)particles.size();
    }
    
    
    
private:
    
    ObjectPool<Particle> elements{};
    std::vector<Particle> particles{};
    ElementBehavior behavior{};
    components::PhysicsComponent physics{};
    
    EmitterType type = EmitterType::et_infinite;
    uint32_t max_size = default_size;
    
    Time elapsed{};
    
    int lifespan{};
    
};

} // end namespace



/* Emitter_hpp */
