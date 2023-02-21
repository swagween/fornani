//
//  Projectile.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <list>
#include "../components/PhysicsComponent.hpp"
#include "../utils/Shape.hpp"
#include "../utils/Random.hpp"
#include "../particle/Emitter.hpp"

namespace arms {

enum class WEAPON_DIR {
    LEFT,
    RIGHT,
    UP_RIGHT,
    UP_LEFT,
    DOWN_RIGHT,
    DOWN_LEFT
};

enum class FIRING_DIRECTION {
    LEFT,
    RIGHT,
    UP,
    DOWN
};

const sf::Vector2<float> DEFAULT_DIMENSIONS{16.0, 16.0};
const int LIFESPAN_VARIANCE = 3;

struct ProjectileStats {
    
    int damage{};
    int lifespan{};
    
    float speed{};
    float variance{};
    float stun{};
    float knockback{};
    
    bool persistent{};
    bool spray{};
    
};

class Projectile {
    
public:
    
    Projectile() {
        physics = components::PhysicsComponent();
        physics.velocity.x = stats.speed;
        seed();
    };
    Projectile(ProjectileStats s, components::PhysicsComponent p) : stats(s), physics(p) {
        physics.velocity.x = stats.speed;
        seed();
    }
    ~Projectile() {}
    
    void update() {
        physics.update();
        bounding_box.vertices[0] = sf::Vector2<float>(physics.position.x,  physics.position.y);
        bounding_box.vertices[1] = sf::Vector2<float>(physics.position.x + DEFAULT_DIMENSIONS.x, physics.position.y);
        bounding_box.vertices[2] = sf::Vector2<float>(physics.position.x + DEFAULT_DIMENSIONS.x, physics.position.y + DEFAULT_DIMENSIONS.y);
        bounding_box.vertices[3] = sf::Vector2<float>(physics.position.x,  physics.position.y + DEFAULT_DIMENSIONS.y);
        bounding_box.update(physics.position.x, physics.position.y, DEFAULT_DIMENSIONS.x, DEFAULT_DIMENSIONS.y);
        stats.lifespan--;
    }
    
    void destroy() {
        stats.lifespan = -1;
    }
    
    void seed() {
        util::Random r{};
        stats.lifespan += r.random_range(-LIFESPAN_VARIANCE, LIFESPAN_VARIANCE);
        float var = r.random_range_float(-stats.variance, stats.variance);
        switch(dir) {
            case FIRING_DIRECTION::LEFT:
                physics.velocity = {-stats.speed + (var/2), var};
                break;
            case FIRING_DIRECTION::RIGHT:
                physics.velocity = {stats.speed + (var/2), var};
                break;
            case FIRING_DIRECTION::UP:
                physics.velocity = {var, -stats.speed + (var/2)};
                break;
            case FIRING_DIRECTION::DOWN:
                physics.velocity = {var, stats.speed + (var/2)};
                break;
        }
    }
    
    FIRING_DIRECTION dir{};
    Shape bounding_box{};
    components::PhysicsComponent physics{};
    ProjectileStats stats{};
    
}; // End Projectile

} // end arms

 /* Projectile_hpp */
