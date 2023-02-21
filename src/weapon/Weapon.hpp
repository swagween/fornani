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

struct WeaponAttributes {
    
    bool automatic{};
    bool boomerang{};
    int rate{};
    int cooldown_time{};
    float recoil{};
    
};

/* ProjectileStats(int dmg, int lfs, float spd, float var, float stn, float kbk, bool per) */
constexpr inline ProjectileStats bryns_gun_stats{    4 , 18, 12.0, 0.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats plasmer_stats{      5 , 18, 16.0, 0.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats wasp_stats{         6 , 24, 8.0 , 0.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats blizzard_stats{     8 , 30, 20.0, 0.0, 1.0, 0.0, false, false};
constexpr inline ProjectileStats bismuth_stats{      8 , 24, 12.0, 0.0, 0.0, 0.0, false, true };
constexpr inline ProjectileStats underdog_stats{     6 , 42, 24.0, 0.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats electron_stats{     9 , 33, 16.0, 0.0, 1.0, 0.0, false, false};
constexpr inline ProjectileStats clover_stats{       4 , 24, 8.0 , 0.5, 0.0, 0.0, false, false};
constexpr inline ProjectileStats triton_stats{       10, 36, 4.0 , 0.0, 0.0, 0.0, true , false};
constexpr inline ProjectileStats willet_585_stats{   9 , 24, 24.0, 0.0, 0.0, 3.0, false, false};
constexpr inline ProjectileStats quasar_stats{       16, 36, 8.0 , 0.0, 0.0, 0.0, false, true };
constexpr inline ProjectileStats nova_stats{         7 , 24, 16.0, 0.0, 0.0, 5.0, false, false};
constexpr inline ProjectileStats venom_stats{        8 , 48, 32.0, 1.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats twin_stats{         7 , 30, 22.0, 0.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats carise_stats{       14, 24, 12.0, 0.0, 4.0, 0.0, false, false};
constexpr inline ProjectileStats stinger_stats{      5 , 30, 16.0, 2.0, 0.0, 0.0, false, false};
constexpr inline ProjectileStats tusk_stats{         16, 24, 20.0, 0.0, 3.0, 0.0, false, false};
constexpr inline ProjectileStats tomahawk_stats{     1 , 20, 2.0 , 0.0, 0.0, 0.0, true , false};


/* WeaponAttributes(bool aut, bool bmr, int rat, int cldn, float recl) */
constexpr inline WeaponAttributes bryns_gun_attributes{  false, false, 3 , 10, 0.0f};
constexpr inline WeaponAttributes plasmer_attributes{    false, false, 3 , 2 , 1.0f};
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
};
struct EmitterStats {
    int lifespan{};
    int lifespan_variance{};
    int particle_lifespan{};
    int particle_lifespan_variance{};
};*/
constexpr inline vfx::ElementBehavior gun_spray{0.8, 2.0, 34.0, 24.5, 1.8, 0.85, 0.6, 0.87, 0.87};
constexpr inline vfx::EmitterStats burst{6, 0, 40, 30};
constexpr inline vfx::ElementBehavior bubble_spray{0.008, 0.1, 0.01, 0.05, 0.3, -0.1, 0.0, 0.85, 0.99};

class Weapon {
    
public:
    
    Weapon() = default;
    Weapon(std::string lbl, WEAPON_TYPE weapon_type, const WeaponAttributes& wa, const ProjectileStats& ps) :
    label(lbl),
    type(weapon_type),
    attributes(wa)
    {
        projectile = Projectile(ps, components::PhysicsComponent());
        spray = vfx::Emitter(gun_spray, burst);
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
                break;
            case WEAPON_DIR::RIGHT:
                projectile.dir = FIRING_DIRECTION::RIGHT;
                break;
            case WEAPON_DIR::UP_RIGHT:
                projectile.dir = FIRING_DIRECTION::UP;
                break;
            case WEAPON_DIR::UP_LEFT:
                projectile.dir = FIRING_DIRECTION::UP;
                break;
            case WEAPON_DIR::DOWN_RIGHT:
                projectile.dir = FIRING_DIRECTION::DOWN;
                break;
            case WEAPON_DIR::DOWN_LEFT:
                projectile.dir = FIRING_DIRECTION::DOWN;
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
    std::string label{};
    
    int current_cooldown = attributes.cooldown_time;
    
private:
    
    bool equipped{};
    bool unlocked{};
    
}; // End AdjacencyList

}

 /* AdjacencyList_hpp */
