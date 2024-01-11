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
#include <unordered_map>
#include "../components/PhysicsComponent.hpp"
#include "../utils/Shape.hpp"
#include "../utils/Random.hpp"
#include "../particle/Emitter.hpp"
#include "../graphics/FLColor.hpp"

namespace arms {

enum class WEAPON_TYPE {
    BRYNS_GUN,
    PLASMER,
    WASP,
    BLIZZARD,
    BISMUTH,
    UNDERDOG,
    ELECTRON,
    CLOVER,
    TRITON,
    WILLET_585,
    QUASAR,
    NOVA,
    VENOM,
    TWIN,
    CARISE,
    STINGER,
    TUSK,
    TOMAHAWK
};

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

const sf::Vector2<float> DEFAULT_DIMENSIONS{8.0, 8.0};

struct ProjectileStats {
    
    int damage{};
    int lifespan{};
    
    float speed{};
    float variance{};
    float stun{};
    float knockback{};
    
    bool persistent{};
    bool spray{};
    
    int lifespan_variance{};
    
};

struct ProjectileAnimation {
    int num_sprites{};
    int num_frames{};
    int framerate{};
};

class Projectile {
    
public:
    
    Projectile() {
        physics = components::PhysicsComponent();
        physics.velocity.x = stats.speed;
        seed();
    };
    Projectile(ProjectileStats s, components::PhysicsComponent p, ProjectileAnimation a, WEAPON_TYPE t) : stats(s), physics(p), anim(a), type(t) {
        physics.velocity.x = stats.speed;
        seed();
        set_sprite();
    }
    ~Projectile() {}
    
    void update() {
        physics.update();
        bounding_box.dimensions = DEFAULT_DIMENSIONS;
        bounding_box.set_position(physics.position);
        stats.lifespan--;
        
        //animation
        if(curr_frame % anim.framerate == 0) {
            anim_frame++;
        }
        if(anim_frame >= anim.num_frames) { anim_frame = 0; }
        curr_frame++;
    }
    
    void destroy() {
        stats.lifespan = -1;
    }
    
    void seed() {
        util::Random r{};
        stats.lifespan += r.random_range(-stats.lifespan_variance, stats.lifespan_variance);
        float var = r.random_range_float(-stats.variance, stats.variance);
        switch(dir) {
            case FIRING_DIRECTION::LEFT:
                physics.velocity = {-stats.speed + (var/2), var};
                physics.dir = components::DIRECTION::LEFT;
                break;
            case FIRING_DIRECTION::RIGHT:
                physics.velocity = {stats.speed + (var/2), var};
                physics.dir = components::DIRECTION::RIGHT;
                break;
            case FIRING_DIRECTION::UP:
                physics.velocity = {var, -stats.speed + (var/2)};
                physics.dir = components::DIRECTION::UP;
                break;
            case FIRING_DIRECTION::DOWN:
                physics.velocity = {var, stats.speed + (var/2)};
                physics.dir = components::DIRECTION::DOWN;
                break;
        }
    }
    
    void set_sprite() {
        if(anim.num_sprites < 2) { sprite_id = 0; return; }
        util::Random r{};
        sprite_id = r.random_range(0, anim.num_sprites - 1 );
    }
    
    FIRING_DIRECTION dir{};
    shape::Shape bounding_box{};
    components::PhysicsComponent physics{};
    ProjectileStats stats{};
    ProjectileAnimation anim{};
    WEAPON_TYPE type{};
    
    int sprite_id{};
    int curr_frame{};
    int anim_frame{};
    
}; // End Projectile

} // end arms

 /* Projectile_hpp */
