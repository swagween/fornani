//
//  Emitter.hpp
//  fornani
//
//  Created by Alex Frasca on 01/05/2023.
//
#pragma once

#include "ObjectPool.hpp"
#include "Particle.hpp"
#include "../components/PhysicsComponent.hpp"
#include <vector>

namespace vfx {

const uint32_t default_size = 64;
using Time = std::chrono::duration<float>;

enum class EMMITER_TYPE {
    ET_INFINITE,
    ET_BURST,
    ET_FADEOUT,
    ET_FADEIN
};

struct ElementBehavior {
    float rate{}; //expulsion rate
    float rate_variance{}; //variance in expulsion rate
    float expulsion_force{};
    float expulsion_variance{};
    float cone{}; //angle in radians of element dispersal
    float grav{};
    float grav_variance{};
    float x_friction{};
    float y_friction{};
};

struct EmitterStats {
    int lifespan{};
    int lifespan_variance{};
    int particle_lifespan{};
    int particle_lifespan_variance{};
};

class Emitter {
    
public:
    
    Emitter() = default;
    Emitter(ElementBehavior b, EmitterStats s, sf::Color c = sf::Color{255, 255, 255}) : behavior(b), stats(s), color(c) {
        util::Random r{};
        int var = r.random_range(-stats.lifespan_variance, stats.lifespan_variance);
        stats.lifespan += var;
    }
    ~Emitter() { particles.clear(); }
    
    void update() { //this will tick every element and the generator itself
        physics.update();
        if(stats.lifespan > 0) { //make a particle at a certain rate
            particles.push_back(Particle(physics, behavior.expulsion_force, behavior.expulsion_variance, behavior.cone, {behavior.x_friction, behavior.y_friction}));
            particles.back().physics.dir = physics.dir;
            util::Random r{};
            int var = r.random_range(-stats.particle_lifespan_variance, stats.particle_lifespan_variance);
            particles.back().lifespan = stats.particle_lifespan + var;
        }
        for(int i = static_cast<int>(particles.size()) - 1; i >= 0; --i) {
            particles.at(i).update(behavior.expulsion_force, behavior.grav, behavior.grav_variance);
            if(particles.at(i).lifespan < 0) {
                particles.erase(particles.begin() + i); //this is bad. I want to delete the element at that spot
            }
        }
        --stats.lifespan;
    }
    
    void set_size(uint32_t sz);
    void set_type(EMMITER_TYPE t);

    bool empty() {
        return particles.empty();
    }
    
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
    
    components::PhysicsComponent& get_physics() { return physics; }
    ElementBehavior& get_behavior() { return behavior; }
    
    void set_rate(float r) { behavior.rate = r; }
    void set_expulsion_force(float f) { behavior.expulsion_force = f; }
    void set_friction(float f) { physics.friction = {f, f}; }
    void set_lifespan(int l) { stats.lifespan = l; }
    void set_direction(components::DIRECTION d) { physics.dir = d; }
    
    std::vector<Particle>& const get_particles() {
        return particles;
    }
    
    int get_lifespan() { return stats.lifespan; }
    int num_particles () { return (int)particles.size(); }
    
    sf::Color color{};
    
    std::vector<Particle> particles{};
    ElementBehavior behavior{};
    EmitterStats stats{};
    components::PhysicsComponent physics{};
    
    EMMITER_TYPE type = EMMITER_TYPE::ET_BURST;
    uint32_t max_size = default_size;
    
};

} // end namespace



/* Emitter_hpp */
