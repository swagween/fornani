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
    WeaponAttributes() = default;
    WeaponAttributes(bool aut, bool bmr, int rat, int cldn, float recl) :
    automatic(aut),
    boomerang(bmr),
    rate(rat),
    cooldown_time(cldn),
    recoil(recl)
    {}
    
    bool automatic{};
    bool boomerang{};
    int rate{};
    int cooldown_time{};
    float recoil{};
};

struct ProjectileStatList {
    /* ProjectileStats(int dmg, int lfs, float spd, float var, float stn, float kbk, bool per) */
    ProjectileStatList() = default;
    ProjectileStats bryns_gun_stats  =  ProjectileStats(4 , 6 , 6.0 , 0.0, 0.0, 0.0, false, false);
    ProjectileStats plasmer_stats    =  ProjectileStats(5 , 6 , 8.0 , 0.0, 0.0, 0.0, false, false);
    ProjectileStats wasp_stats       =  ProjectileStats(6 , 8 , 4.0 , 0.0, 0.0, 0.0, false, false);
    ProjectileStats blizzard_stats   =  ProjectileStats(8 , 10, 10.0, 0.0, 1.0, 0.0, false, false);
    ProjectileStats bismuth_stats    =  ProjectileStats(8 , 8 , 6.0 , 0.0, 0.0, 0.0, false, true );
    ProjectileStats underdog_stats   =  ProjectileStats(6 , 14, 12.0, 0.0, 0.0, 0.0, false, false);
    ProjectileStats electron_stats   =  ProjectileStats(9 , 11, 8.0 , 0.0, 1.0, 0.0, false, false);
    ProjectileStats clover_stats     =  ProjectileStats(4 , 8 , 4.0 , 3.0, 0.0, 0.0, false, false);
    ProjectileStats triton_stats     =  ProjectileStats(10, 12, 2.0 , 0.0, 0.0, 0.0, true , false);
    ProjectileStats willet_585_stats =  ProjectileStats(9 , 8 , 12.0, 0.0, 0.0, 3.0, false, false);
    ProjectileStats quasar_stats     =  ProjectileStats(16, 12, 4.0 , 0.0, 0.0, 0.0, false, true );
    ProjectileStats nova_stats       =  ProjectileStats(7 , 8 , 8.0 , 0.0, 0.0, 5.0, false, false);
    ProjectileStats venom_stats      =  ProjectileStats(8 , 16, 16.0, 1.0, 0.0, 0.0, false, false);
    ProjectileStats twin_stats       =  ProjectileStats(7 , 10, 11.0, 0.0, 0.0, 0.0, false, false);
    ProjectileStats carise_stats     =  ProjectileStats(14, 8 , 6.0 , 0.0, 4.0, 0.0, false, false);
    ProjectileStats stinger_stats    =  ProjectileStats(5 , 10, 8.0 , 2.0, 0.0, 0.0, false, false);
    ProjectileStats tusk_stats       =  ProjectileStats(16, 8 , 10.0, 0.0, 3.0, 0.0, false, false);
    ProjectileStats tomahawk_stats   =  ProjectileStats(1 , 8 , 1.0 , 0.0, 0.0, 0.0, true , false);
};

struct WeaponAttributeList {
    /* WeaponAttributes(bool aut, bool bmr, int rat, int cldn, float recl) */
    WeaponAttributeList() = default;
    WeaponAttributes bryns_gun_attributes  =  WeaponAttributes(false, false, 3 , 2 , 0.0f);
    WeaponAttributes plasmer_attributes    =  WeaponAttributes(false, false, 3 , 2 , 1.0f);
    WeaponAttributes wasp_attributes       =  WeaponAttributes(false, false, 4 , 1 , 0.0f);
    WeaponAttributes blizzard_attributes   =  WeaponAttributes(false, false, 2 , 3 , 0.0f);
    WeaponAttributes bismuth_attributes    =  WeaponAttributes(false, false, 3 , 2 , 2.0f);
    WeaponAttributes underdog_attributes   =  WeaponAttributes(false, false, 6 , 1 , 0.0f);
    WeaponAttributes electron_attributes   =  WeaponAttributes(false, false, 2 , 4 , 0.0f);
    WeaponAttributes clover_attributes     =  WeaponAttributes(true , false, 16, 4 , 0.0f);
    WeaponAttributes triton_attributes     =  WeaponAttributes(false, false, 2 , 0 , 0.0f);
    WeaponAttributes willet_585_attributes =  WeaponAttributes(false, false, 3 , 1 , 0.0f);
    WeaponAttributes quasar_attributes     =  WeaponAttributes(false, false, 1 , 16, 0.0f);
    WeaponAttributes nova_attributes       =  WeaponAttributes(false, false, 4 , 3 , 0.0f);
    WeaponAttributes venom_attributes      =  WeaponAttributes(false, false, 3 , 5 , 0.0f);
    WeaponAttributes twin_attributes       =  WeaponAttributes(false, false, 4 , 4 , 3.0f);
    WeaponAttributes carise_attributes     =  WeaponAttributes(false, false, 4 , 2 , 0.0f);
    WeaponAttributes stinger_attributes    =  WeaponAttributes(true , false, 16, 2 , 0.0f);
    WeaponAttributes tusk_attributes       =  WeaponAttributes(false, false, 2 , 5 , 0.0f);
    WeaponAttributes tomahawk_attributes   =  WeaponAttributes(false, true , 1 , 0 , 0.0f);
};

static const ProjectileStatList psl{};
static const WeaponAttributeList wal{};

class Weapon {
    
public:
    
    Weapon() = default;
    Weapon(std::string lbl, WEAPON_TYPE weapon_type, WeaponAttributes wa, ProjectileStats ps) : label(lbl), type(weapon_type), attributes(wa) {
        projectile = Projectile(ps, components::PhysicsComponent());
    }
    ~Weapon() { ammo.clear(); }
    
    WeaponAttributes attributes{};
    
    Projectile projectile{};
    std::vector<Projectile> ammo;
    
    WEAPON_TYPE type;
    std::string label;
    
    
    
}; // End AdjacencyList

}

 /* AdjacencyList_hpp */
