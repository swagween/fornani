//
//  Weapon.hpp
//  fornani
//
//  Created by Alex Frasca on 12/26/22.
//

#pragma once

#include <cstdio>
#include <memory>
#include <list>
#include "Projectile.hpp"

namespace arms {

struct WeaponAttributes {
    
    bool automatic{};
    bool boomerang{};
    int rate{};
    int cooldown_time{};
    float recoil{};
    
};

inline std::unordered_map<WEAPON_DIR, int> WeaponDirLookup {
    {WEAPON_DIR::LEFT,          1},
    {WEAPON_DIR::RIGHT,         0},
    {WEAPON_DIR::UP_RIGHT,      2},
    {WEAPON_DIR::UP_LEFT,       5},
    {WEAPON_DIR::DOWN_RIGHT,    3},
    {WEAPON_DIR::DOWN_LEFT,     4}
};

inline std::unordered_map<FIRING_DIRECTION, int> ProjDirLookup {
    {FIRING_DIRECTION::LEFT,    1},
    {FIRING_DIRECTION::RIGHT,   0},
    {FIRING_DIRECTION::UP,      2},
    {FIRING_DIRECTION::DOWN,    3},
};


inline std::unordered_map<arms::WEAPON_TYPE, sf::Color> spray_color {
    {arms::WEAPON_TYPE::BRYNS_GUN,  flcolor::periwinkle },
    {arms::WEAPON_TYPE::PLASMER,    flcolor::fucshia    },
    {arms::WEAPON_TYPE::WASP,       flcolor::goldenrod  },
    {arms::WEAPON_TYPE::BLIZZARD,   flcolor::blue       },
    {arms::WEAPON_TYPE::BISMUTH,    flcolor::fucshia    },
    {arms::WEAPON_TYPE::UNDERDOG,   flcolor::orange     },
    {arms::WEAPON_TYPE::ELECTRON,   flcolor::white      },
    {arms::WEAPON_TYPE::CLOVER,     flcolor::green      },
    {arms::WEAPON_TYPE::TRITON,     flcolor::goldenrod  },
    {arms::WEAPON_TYPE::WILLET_585, flcolor::periwinkle },
    {arms::WEAPON_TYPE::QUASAR,     flcolor::green      },
    {arms::WEAPON_TYPE::NOVA,       flcolor::white      },
    {arms::WEAPON_TYPE::VENOM,      flcolor::goldenrod  },
    {arms::WEAPON_TYPE::TWIN,       flcolor::fucshia    },
    {arms::WEAPON_TYPE::CARISE,     flcolor::blue       },
    {arms::WEAPON_TYPE::STINGER,    flcolor::goldenrod  },
    {arms::WEAPON_TYPE::TUSK,       flcolor::white      },
    {arms::WEAPON_TYPE::TOMAHAWK,   flcolor::white      }
};

constexpr inline ProjectileAnimation default_anim{1, 1, 8};
constexpr inline ProjectileAnimation clover_anim{5, 4, 4};

/* ProjectileStats(int dmg, int lfs, float spd, float var, float stn, float kbk, bool per, int lifespan_var) */
constexpr inline ProjectileStats bryns_gun_stats{    4 , 18, 4.0, 0.1, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats plasmer_stats{      5 , 12, 8.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats wasp_stats{         6 , 24, 8.0 , 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats blizzard_stats{     8 , 30, 20.0, 0.0, 1.0, 0.0, false, false, 0};
constexpr inline ProjectileStats bismuth_stats{      8 , 24, 12.0, 0.2, 0.0, 0.0, false, true , 0};
constexpr inline ProjectileStats underdog_stats{     6 , 42, 24.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats electron_stats{     9 , 33, 16.0, 0.0, 1.0, 0.0, false, false, 0};
constexpr inline ProjectileStats clover_stats{       4 , 44, 1.5 , 0.1, 0.0, 0.0, false, false, 4};
constexpr inline ProjectileStats triton_stats{       10, 36, 4.0 , 0.0, 0.0, 0.0, true , false, 0};
constexpr inline ProjectileStats willet_585_stats{   9 , 24, 24.0, 0.0, 0.0, 3.0, false, false, 0};
constexpr inline ProjectileStats quasar_stats{       16, 36, 8.0 , 0.0, 0.0, 0.0, false, true , 0};
constexpr inline ProjectileStats nova_stats{         7 , 24, 16.0, 0.0, 0.0, 5.0, false, false, 0};
constexpr inline ProjectileStats venom_stats{        8 , 48, 32.0, 1.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats twin_stats{         7 , 30, 22.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats carise_stats{       14, 24, 12.0, 0.1, 4.0, 0.0, false, false, 0};
constexpr inline ProjectileStats stinger_stats{      5 , 30, 16.0, 2.0, 0.0, 0.0, false, false, 3};
constexpr inline ProjectileStats tusk_stats{         16, 24, 20.0, 0.0, 3.0, 0.0, false, false, 0};
constexpr inline ProjectileStats tomahawk_stats{     1 , 20, 2.0 , 0.0, 0.0, 0.0, true , false, 0};


/* WeaponAttributes(bool aut, bool bmr, int rat, int cldn, float recl) */
constexpr inline WeaponAttributes bryns_gun_attributes{  false, false, 4 , 1,  0.0f};
constexpr inline WeaponAttributes plasmer_attributes{    false, false, 3 , 2 , 0.05f};
constexpr inline WeaponAttributes wasp_attributes{       false, false, 4 , 1 , 0.0f};
constexpr inline WeaponAttributes blizzard_attributes{   false, false, 2 , 3 , 0.0f};
constexpr inline WeaponAttributes bismuth_attributes{    false, false, 3 , 2 , 2.0f};
constexpr inline WeaponAttributes underdog_attributes{   false, false, 6 , 1 , 0.0f};
constexpr inline WeaponAttributes electron_attributes{   false, false, 2 , 4 , 0.0f};
constexpr inline WeaponAttributes clover_attributes{     true , false, 16, 8 , 0.0f};
constexpr inline WeaponAttributes triton_attributes{     false, false, 2 , 0 , 0.0f};
constexpr inline WeaponAttributes willet_585_attributes{ false, false, 3 , 1 , 0.0f};
constexpr inline WeaponAttributes quasar_attributes{     false, false, 1 , 16, 0.0f};
constexpr inline WeaponAttributes nova_attributes{       false, false, 4 , 3 , 0.0f};
constexpr inline WeaponAttributes venom_attributes{      false, false, 3 , 5 , 0.0f};
constexpr inline WeaponAttributes twin_attributes{       false, false, 4 , 4 , 3.0f};
constexpr inline WeaponAttributes carise_attributes{     false, false, 4 , 2 , 0.0f};
constexpr inline WeaponAttributes stinger_attributes{    true , false, 16, 2 , 0.0f};
constexpr inline WeaponAttributes tusk_attributes{       false, false, 2 , 5 , 0.0f};
constexpr inline WeaponAttributes tomahawk_attributes{   false, true , 1 , 0 , 0.0f};

/* struct ElementBehavior {
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
};*/
constexpr inline vfx::ElementBehavior heavy_gun_spray{2, 1, 1.0, 0.8, 0.8, 0.005, 0.001, 0.99, 0.99};
constexpr inline vfx::ElementBehavior light_gun_spray{2, 3, 1.9, 0.8, 0.4, 0.0, 0.0, 0.99, 0.99};
constexpr inline vfx::ElementBehavior bubble_spray{1, 0, 1.3, 1.5, 3.4, -0.01, 0.001, 0.92, 0.92};
constexpr inline vfx::EmitterStats burst{6, 0, 40, 30};

class Weapon {
    
public:
    
    Weapon() = default;
    Weapon(std::string lbl, WEAPON_TYPE weapon_type, const WeaponAttributes& wa, const ProjectileStats& ps, const vfx::ElementBehavior spr = light_gun_spray, const ProjectileAnimation& pa = default_anim, sf::Vector2<int> dim = {16, 16}) :
    label(lbl),
    type(weapon_type),
    attributes(wa),
    sprite_dimensions(dim)
    {
        projectile = Projectile(ps, components::PhysicsComponent(), pa, type);
        spray = vfx::Emitter(spr, burst, spray_color.at(type));
    }
    ~Weapon() {}
    
    void equip()    { equipped = true ; }
    void unequip()  { equipped = false; }
    void unlock()   { unlocked = true ; }
    void lock()     { unlocked = true ; }
    
    bool is_equipped() { return equipped; }
    bool is_unlocked() { return unlocked; }
    
    void set_position(sf::Vector2<float> pos) { sprite_position = pos; }
    void set_orientation() {
        switch(sprite_orientation) {
            case WEAPON_DIR::LEFT:
                projectile.dir = FIRING_DIRECTION::LEFT;
                sprite_offset = {-sprite_dimensions.x, -sprite_dimensions.y/4};
                break;
            case WEAPON_DIR::RIGHT:
                projectile.dir = FIRING_DIRECTION::RIGHT;
                sprite_offset = {0, -sprite_dimensions.y/4};
                break;
            case WEAPON_DIR::UP_RIGHT:
                projectile.dir = FIRING_DIRECTION::UP;
                sprite_offset = {0, -sprite_dimensions.x};
                break;
            case WEAPON_DIR::UP_LEFT:
                projectile.dir = FIRING_DIRECTION::UP;
                sprite_offset = {-sprite_dimensions.y/2, -sprite_dimensions.x};
                break;
            case WEAPON_DIR::DOWN_RIGHT:
                projectile.dir = FIRING_DIRECTION::DOWN;
                sprite_offset = {-sprite_dimensions.y/2, 0};
                break;
            case WEAPON_DIR::DOWN_LEFT:
                projectile.dir = FIRING_DIRECTION::DOWN;
                sprite_offset = {0, 0};
                break;
        }
    }
    
    WeaponAttributes attributes{};
    
    Projectile projectile{};
    vfx::Emitter spray{};
    
    sf::Vector2<float> sprite_position{};
    std::vector<sf::Vector2<float>> anchor_points{};
    
    WEAPON_TYPE type{};
    WEAPON_DIR sprite_orientation{};
    sf::Vector2<int> sprite_dimensions{};
    sf::Vector2<int> sprite_offset{};
    std::string label{};
    
    int current_cooldown = attributes.cooldown_time;
    
private:
    
    bool equipped{};
    bool unlocked{};
    
}; // End AdjacencyList

}

 /* AdjacencyList_hpp */
