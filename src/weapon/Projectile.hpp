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
#include <chrono>
#include <deque>
#include "../components/PhysicsComponent.hpp"
#include "../entities/behavior/Animation.hpp"
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
const int history_limit{ 12 };

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

    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;
    
public:
    
    Projectile();
    Projectile(ProjectileStats s, components::PhysicsComponent p, ProjectileAnimation a, WEAPON_TYPE t);

    void update();
    void render(sf::RenderWindow& win, sf::Vector2<float>& campos);
    void destroy();
    void seed();
    void set_sprite();
    
    FIRING_DIRECTION dir{};
    shape::Shape bounding_box{};
    components::PhysicsComponent physics{};
    ProjectileStats stats{};
    ProjectileAnimation anim{};
    WEAPON_TYPE type{WEAPON_TYPE::BRYNS_GUN};

    std::vector<sf::Sprite> sp_proj{};
    
    int sprite_id{};
    int curr_frame{};
    int anim_frame{};

    //fixed animation time step variables
    Time dt{ 0.001f };
    Clock::time_point current_time = Clock::now();
    Time accumulator{ 0.0f };

    sf::RectangleShape box{};

    std::deque<sf::Vector2<float>> position_history{};
    
}; // End Projectile

} // end arms

 /* Projectile_hpp */
