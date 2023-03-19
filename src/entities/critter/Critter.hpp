//
//  Critter.hpp
//  critter
//
//

#pragma once

#include <vector>
#include <cmath>
#include <random>
#include <list>

namespace critter {

struct CritterMetadata {
    int id{};
};

struct CritterStats {
    int base_damage{};
    int base_hp{};
    float speed{};
    float loot_multiplier{};
    
};

struct CritterFlags {
    bool hostile{};
    bool hurt_on_contact{};
    bool alive{};
};

class Critter {
    
public:
    Critter() = default;
    Critter(CritterMetadata m, CritterStats s, CritterBehaviorComponent b, std::vector<CritterActionComponent> c);
    virtual void update();
    virtual void render(sf::RenderWindow& win, sf::Vector2<float> campos);
    
    components::CritterBehaviorComponent behavior{};
    components::PhysicsComponent physics{};
    std::vector<components::CritterActionComponent> actions{};
    
    
};


} // end critter

/* Critter_hpp */
