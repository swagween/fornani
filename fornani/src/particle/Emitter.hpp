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
    int rate{}; //expulsion rate
    int rate_variance{}; //variance in expulsion rate
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
    float part_size = 3.0f;
};

class Emitter {
    
public:
    
    Emitter() = default;
    Emitter(ElementBehavior b, EmitterStats s, sf::Color c = sf::Color{ 255, 255, 255 });
    ~Emitter();
    
    void update();
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);

    bool empty();
    
    void set_position(float x, float y);
    void set_velocity(float x, float y);
    
    void apply_force(sf::Vector2<float> force);
    void apply_force_at_angle(float force, float angle);
    
    components::PhysicsComponent& get_physics();
    ElementBehavior& get_behavior();
    
    void set_rate(float r);
    void set_expulsion_force(float f);
    void set_friction(float f);
    void set_lifespan(int l);
    void set_direction(components::DIRECTION d);
    
    std::vector<Particle>& const get_particles();
    
    sf::Color color{};
	sf::RectangleShape dot{};
    
    std::vector<Particle> particles{};
    ElementBehavior behavior{};
    EmitterStats stats{};
    components::PhysicsComponent physics{};
    
    EMMITER_TYPE type = EMMITER_TYPE::ET_BURST;
    uint32_t max_size = default_size;
    
};

} // end namespace



/* Emitter_hpp */
