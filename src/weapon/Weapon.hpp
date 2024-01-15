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

    enum COLOR_CODE {
        WHITE = 0,
        PERIWINKLE = 1,
        GREEN = 2,
        ORANGE = 3,
        FUCSHIA = 4,
        PURPLE = 5
    };

struct WeaponAttributes {
    
    bool automatic{};
    bool boomerang{};
    int rate{};
    int cooldown_time{};
    float recoil{};
    COLOR_CODE ui_color{};
    std::array<float, 2> barrel_position{};
    
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
constexpr inline ProjectileAnimation bg_anim{ 4, 1, 8 };

/* ProjectileStats(int dmg, int lfs, float spd, float var, float stn, float kbk, bool per, int lifespan_var) */
constexpr inline ProjectileStats bryns_gun_stats{    4 , 320, 2.5, 0.1, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats plasmer_stats{      5 , 220, 3.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats wasp_stats{         6 , 240, 8.0 , 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats blizzard_stats{     8 , 300, 20.0, 0.0, 1.0, 0.0, false, false, 0};
constexpr inline ProjectileStats bismuth_stats{      8 , 240, 12.0, 0.2, 0.0, 0.0, false, true , 0};
constexpr inline ProjectileStats underdog_stats{     6 , 420, 24.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats electron_stats{     9 , 330, 16.0, 0.0, 1.0, 0.0, false, false, 0};
constexpr inline ProjectileStats clover_stats{       4 , 440, 0.5 , 0.1, 0.0, 0.0, false, false, 4};
constexpr inline ProjectileStats triton_stats{       10, 360, 4.0 , 0.0, 0.0, 0.0, true , false, 0};
constexpr inline ProjectileStats willet_585_stats{   9 , 240, 24.0, 0.0, 0.0, 3.0, false, false, 0};
constexpr inline ProjectileStats quasar_stats{       16, 360, 8.0 , 0.0, 0.0, 0.0, false, true , 0};
constexpr inline ProjectileStats nova_stats{         7 , 240, 16.0, 0.0, 0.0, 5.0, false, false, 0};
constexpr inline ProjectileStats venom_stats{        8 , 480, 32.0, 1.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats twin_stats{         7 , 300, 22.0, 0.0, 0.0, 0.0, false, false, 0};
constexpr inline ProjectileStats carise_stats{       14, 240, 12.0, 0.1, 4.0, 0.0, false, false, 0};
constexpr inline ProjectileStats stinger_stats{      5 , 300, 16.0, 2.0, 0.0, 0.0, false, false, 3};
constexpr inline ProjectileStats tusk_stats{         16, 240, 20.0, 0.0, 3.0, 0.0, false, false, 0};
constexpr inline ProjectileStats tomahawk_stats{     1 , 200, 2.0 , 0.0, 0.0, 0.0, true , false, 0};

//barrel points on sprites
constexpr inline std::array<float, 2> bg_barrel{ 18.0f, 2.0f };
constexpr inline std::array<float, 2> plasmer_barrel{ 22.0f, 7.0f };
constexpr inline std::array<float, 2> clover_barrel{ 18.0f, 4.0f };


/* WeaponAttributes(bool aut, bool bmr, int rat, int cldn, float recl) */
constexpr inline WeaponAttributes bryns_gun_attributes{  false, false, 4 , 1,  0.0f, PERIWINKLE,    bg_barrel };
constexpr inline WeaponAttributes plasmer_attributes{    false, false, 3 , 2 , 0.1f, FUCSHIA,       plasmer_barrel };
constexpr inline WeaponAttributes wasp_attributes{       false, false, 4 , 1 , 0.0f, ORANGE,        bg_barrel };
constexpr inline WeaponAttributes blizzard_attributes{   false, false, 2 , 3 , 0.0f, PERIWINKLE,    bg_barrel };
constexpr inline WeaponAttributes bismuth_attributes{    false, false, 3 , 2 , 2.0f, FUCSHIA,       bg_barrel };
constexpr inline WeaponAttributes underdog_attributes{   false, false, 6 , 1 , 0.0f, ORANGE,        bg_barrel };
constexpr inline WeaponAttributes electron_attributes{   false, false, 2 , 4 , 0.0f, WHITE,         bg_barrel };
constexpr inline WeaponAttributes clover_attributes{     true , false, 16, 8 , 0.0f, GREEN,         clover_barrel };
constexpr inline WeaponAttributes triton_attributes{     false, false, 2 , 0 , 0.0f, ORANGE,        bg_barrel };
constexpr inline WeaponAttributes willet_585_attributes{ false, false, 3 , 1 , 0.0f, PURPLE,        bg_barrel };
constexpr inline WeaponAttributes quasar_attributes{     false, false, 1 , 16, 0.0f, GREEN,         bg_barrel };
constexpr inline WeaponAttributes nova_attributes{       false, false, 4 , 3 , 0.0f, GREEN,         bg_barrel };
constexpr inline WeaponAttributes venom_attributes{      false, false, 3 , 5 , 0.0f, ORANGE,        bg_barrel };
constexpr inline WeaponAttributes twin_attributes{       false, false, 4 , 4 , 3.0f, FUCSHIA,       bg_barrel };
constexpr inline WeaponAttributes carise_attributes{     false, false, 4 , 2 , 0.0f, PURPLE,        bg_barrel };
constexpr inline WeaponAttributes stinger_attributes{    true , false, 16, 2 , 0.0f, WHITE,         bg_barrel };
constexpr inline WeaponAttributes tusk_attributes{       false, false, 2 , 5 , 0.0f, WHITE,         bg_barrel };
constexpr inline WeaponAttributes tomahawk_attributes{   false, true , 1 , 0 , 0.0f, WHITE,         bg_barrel };

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
    Weapon(std::string lbl, WEAPON_TYPE weapon_type, const WeaponAttributes& wa, const ProjectileStats& ps, 
        const vfx::ElementBehavior spr = light_gun_spray, const ProjectileAnimation& pa = default_anim, 
        sf::Vector2<int> dim = { 16, 16 });

    void update();
    void render(sf::RenderWindow& win, sf::Vector2<float>& campos);

    void equip();
    void unequip();
    void unlock();
    void lock();

    bool is_equipped() const;
    bool is_unlocked() const;

    void set_position(sf::Vector2<float> pos);
    void set_orientation();
    
    WeaponAttributes attributes{};
    
    Projectile projectile{};
    vfx::Emitter spray{};
    
    sf::Vector2<float> sprite_position{};
    std::vector<sf::Vector2<float>> anchor_points{};
    sf::Vector2<float> barrel_point{};
    
    WEAPON_TYPE type{};
    sf::Vector2<int> sprite_dimensions{};
    sf::Vector2<int> sprite_offset{};
    std::string label{};

    sf::Sprite sp_gun{};
    
    int current_cooldown = attributes.cooldown_time;
    FIRING_DIRECTION fire_dir{};

    
private:
    
    bool equipped{};
    bool unlocked{};
    
}; // End AdjacencyList

}

 /* AdjacencyList_hpp */
