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
#include <unordered_map>
#include <queue>
#include "../../setup/ServiceLocator.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "../../utils/Shape.hpp"

namespace critter {

inline std::unordered_map<int, sf::Texture&> get_critter_texture {
    {0, svc::assetLocator.get().t_frdog}
};

struct CritterMetadata {
    int id{};
    int variant{};
    bool hostile{};
    bool hurt_on_contact{};
    bool gravity{};
};

struct CritterStats {
    int base_damage{};
    int base_hp{};
    float speed{};
    float loot_multiplier{};
    
    int energy{};
    
};

struct CritterFlags {
    bool alive{};
    bool seeking{};
};

struct FrameTracker {
    int frame{};
    
};

class Critter {
    
public:
    Critter() = default;
    Critter(CritterMetadata m, CritterStats s, sf::Vector2<int> sprite_dim, sf::Vector2<int> spritesheet_dim, sf::Vector2<float> dim) : metadata(m), stats(s), sprite_dimensions(sprite_dim), spritesheet_dimensions(spritesheet_dim), dimensions(dim) {
        bounding_box = Shape{dim};
        set_sprite();
        physics = components::PhysicsComponent(sf::Vector2<float>{0.6f, 0.98f}, 1.0f);
    }
    ~Critter() {}
    
    void update();
    void render(sf::RenderWindow& win, sf::Vector2<float> campos);
    void handle_map_collision(const Shape& cell, bool is_ramp);
    void set_sprite();
    void set_position(sf::Vector2<int> pos);
    void seek_current_target();
    
    //general critter methods, to be called dependent on critter type
//    void stop();
//    void reverse_direction();
    void random_idle_action();
    void random_walk(sf::Vector2<int> range);
//    void attack();
//    void defend();
//    void hop();
//    void leap();
//    void evade(sf::Vector2<float>& stimulus_pos);
//    void hover_sine();
//    void fly_sine();
//    void fly_line(bool vertical);
    
    CritterMetadata metadata{};
    CritterStats stats{};
    CritterFlags flags{};
    
    components::CritterBehaviorComponent behavior{};
    components::PhysicsComponent physics{};
    
    Shape bounding_box{};
    
    sf::Vector2<int> sprite_dimensions{};
    sf::Vector2<int> spritesheet_dimensions{};
    sf::Vector2<float> dimensions{};
    
    sf::Sprite sprite{};
    sf::RectangleShape hurtbox{}; // for debugging
    
    std::queue<int> idle_action_queue{};
    
    sf::Vector2<float> current_target{};
    
};


} // end critter

/* Critter_hpp */
