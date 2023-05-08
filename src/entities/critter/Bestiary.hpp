//
//  Bestiary.hpp
//  critter
//
//

#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <list>
#include "Critter.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "critters/Frdog.hpp"
#include "critters/Hulmet.hpp"

namespace critter {

    /*
    CritterMetadata { id, variant, hostile, hurt_on_contact, gravity }
    CritterStats    { base_damage, base_hp, speed, loot_multiplier, energy, vision }
    */

inline Critter frdog = Critter(CritterMetadata{0, VARIANT::BEAST, true, false, true}, CritterStats{3, 35, 0.3f, 2.0f, 80, 12 * 32}, sf::Vector2<int>{72, 48}, sf::Vector2<int>{4, 5}, sf::Vector2<float>{32.0f, 32.0f});
inline Critter hulmet = Critter(CritterMetadata{1, VARIANT::SOLDIER, true, false, true }, CritterStats{ 2, 14, 0.5f, 1.0f, 16 * 32 }, sf::Vector2<int>{56, 42}, sf::Vector2<int>{1, 14}, sf::Vector2<float>{16.0f, 16.0f});

inline Frdog c_frdog{};
inline Hulmet c_hulmet{};


struct BestiaryList {
    
    BestiaryList() {
        critters.insert( {frdog.metadata.id, c_frdog} );
        critters.insert( {hulmet.metadata.id, c_hulmet});
    }
    
    std::unordered_map<int, Critter&> critters{};
};

class Bestiary {
public:
    Bestiary() = default;
    ~Bestiary() { bestiary_list.critters.clear(); }
    Bestiary& operator=(Bestiary&&) = delete;
    
    Critter& get_critter_at(int idx) { return bestiary_list.critters.at(idx); }
    
    BestiaryList bestiary_list{};
};


} // end critter

/* Bestiary_hpp */
