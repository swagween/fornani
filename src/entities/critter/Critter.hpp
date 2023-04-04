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
#include "../../utils/Collider.hpp"

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
    int vision{};
    
};

struct CritterFlags {
    bool alive{};
    bool seeking{};
    bool awake{};
    bool awakened{};
    bool asleep{};
};

struct FrameTracker {
    int frame{};
    
};

class Critter {
    
public:
    Critter() = default;
    Critter(CritterMetadata m, CritterStats s, sf::Vector2<int> sprite_dim, sf::Vector2<int> spritesheet_dim, sf::Vector2<float> dim) : metadata(m), stats(s), sprite_dimensions(sprite_dim), spritesheet_dimensions(spritesheet_dim), dimensions(dim) {
        collider = shape::Collider(); 
        set_sprite();
        collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.997f}, 1.0f);
        collider.physics.maximum_velocity = sf::Vector2<float>(stats.speed, stats.speed*4);
        if (metadata.gravity) { collider.physics.gravity = 0.03f; }

        alert_range = Shape( { (float)s.vision * 1.5f, (float)s.vision * 1.5f } );
        hostile_range = Shape( { (float)s.vision, (float)s.vision } );

        ar.setSize( { (float)(s.vision * 1.5), (float)(s.vision * 1.5) });
        hr.setSize( { (float)s.vision, (float)s.vision } );
    }
    ~Critter() {}
    
    void update();
    void render(sf::RenderWindow& win, sf::Vector2<float> campos);
    void set_sprite();
    void set_position(sf::Vector2<int> pos);
    void seek_current_target();
    void wake_up();
    void sleep();
    void awake();

    
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
    shape::Collider collider{};
    Shape alert_range{};
    Shape hostile_range{};
    
    sf::Vector2<int> sprite_dimensions{};
    sf::Vector2<int> spritesheet_dimensions{};
    sf::Vector2<float> dimensions{};
    
    sf::Sprite sprite{};
    sf::RectangleShape hurtbox{}; // for debugging
    sf::RectangleShape ar{};
    sf::RectangleShape hr{};
    
    std::queue<int> idle_action_queue{};
    
    sf::Vector2<float> current_target{};
    
};


} // end critter

/* Critter_hpp */
