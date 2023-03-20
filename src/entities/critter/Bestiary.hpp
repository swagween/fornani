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

namespace critter {

inline Critter frdog = Critter(CritterMetadata{0, 0, true, false, true}, CritterStats{3, 35, 2, 2}, sf::Vector2<int>{72, 48}, sf::Vector2<int>{4, 5}, sf::Vector2<float>{32.0f, 32.0f});

struct BestiaryList {
    
    BestiaryList() {
        critters.insert( {frdog.metadata.id, frdog} );
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
