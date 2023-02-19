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

namespace arms {

struct ProjectileStats {
    
    ProjectileStats() = default;
    ProjectileStats(int dmg, int lfs, float spd, float var, float stn, float kbk, bool per, bool spr) :
    damage(dmg),
    lifespan(lfs),
    speed(spd),
    variance(var),
    stun(stn),
    knockback(kbk),
    persistent(per),
    spray(spr)
    {}
    
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
    
    Projectile() = default;
    Projectile(ProjectileStats s, components::PhysicsComponent p) : stats(s), physics(p) {}
    ~Projectile() {}
    
    components::PhysicsComponent physics{};
    ProjectileStats stats{};
    
}; // End Projectile

} // end arms

 /* Projectile_hpp */
