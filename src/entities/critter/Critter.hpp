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
#include <assert.h>
#include "../../setup/ServiceLocator.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/StateFunction.hpp"
#include <djson/json.hpp>

namespace critter {

    enum class VARIANT {
        BEAST,
        SOLDIER,
        GRUB,
        GHOST
    };

    enum class CRITTER_TYPE {
        hulmet,
        tank,
        bunker,
        thug,
        worker,
        frdog,
        jackdaw,
        frcat,
        biter,
        bee,
        roller,
        snek,
        behemoth,
        stinger,
        watchman,
        gnat,
        moth
    };

inline std::unordered_map<int, sf::Texture&> get_critter_texture {
    {0, svc::assetLocator.get().t_frdog},
    {1, svc::assetLocator.get().t_hulmet}
};

struct CritterMetadata {
    int id{};
    VARIANT variant{};
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

struct CritterCondition {
    int hp{1};
};

struct CritterFlags {
    bool alive{};
    bool seeking{};
    bool awake{};
    bool awakened{};
    bool asleep{};
    bool turning{};
    bool flip{};
    bool barking{};
    bool hurt{};
    bool just_hurt{};
    bool shot{};
    bool vulnerable{};
};

struct FrameTracker {
    int frame{};
    
};

class Critter {
    
public:

    Critter() = default;
    Critter(CritterMetadata m, CritterStats s, sf::Vector2<int> sprite_dim, sf::Vector2<int> spritesheet_dim, sf::Vector2<float> dim) : metadata(m), stats(s), sprite_dimensions(sprite_dim), spritesheet_dimensions(spritesheet_dim), dimensions(dim) {
        //colliders.push_back( shape::Collider() ); 
        set_sprite();
        /*colliders.at(0).physics = components::PhysicsComponent(sf::Vector2<float>{0.8f, 0.997f}, 1.0f);
        colliders.at(0).physics.maximum_velocity = sf::Vector2<float>(s.speed, s.speed*4);*/
        //if (m.gravity) { colliders.at(0).physics.gravity = 0.03f; }

        alert_range = shape::Shape( { (float)s.vision * 1.5f, (float)s.vision * 1.5f } );
        hostile_range = shape::Shape( { (float)s.vision, (float)s.vision } );

        ar.setSize( { (float)(s.vision * 1.5), (float)(s.vision * 1.5) });
        hr.setSize( { (float)s.vision, (float)s.vision } );
        condition.hp = s.base_hp;
    }
    ~Critter() {}

    virtual void unique_update() {};
    virtual void load_data() {};
    virtual void sprite_flip();
    
    void init();
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
    CritterCondition condition{};
    CritterFlags flags{};
    
    behavior::Behavior behavior{};
    behavior::DIR_LR facing_lr{};
    std::vector<shape::Collider> colliders{};

    std::vector<shape::Shape> hurtbox_atlas{};
    std::vector<shape::Shape> hurtboxes{};
    shape::Shape alert_range{};
    shape::Shape hostile_range{};
    
    sf::Vector2<int> sprite_dimensions{};
    sf::Vector2<int> spritesheet_dimensions{};
    sf::Vector2<float> offset{};
    sf::Vector2<float> dimensions{};
    sf::Vector2<float> sprite_position{};
    int anim_loop_count{};

    //collider loading
    int num_anim_frames{};
    int num_colliders{};
    int num_hurtboxes{};
    
    sf::Sprite sprite{};
    sf::RectangleShape drawbox{}; // for debugging
    sf::RectangleShape ar{};
    sf::RectangleShape hr{};
    sf::RectangleShape hpbox{}; // for debug
    
    std::queue<int> idle_action_queue{};
    
    sf::Vector2<float> current_target{};
    
};


} // end critter

/* Critter_hpp */
