//
//  Player.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//
#pragma once

#include "../../utils/Shape.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "../../weapon/Arsenal.hpp"
#include <array>
#include <memory>

const float PLAYER_WIDTH = 16.0f;
const float PLAYER_HEIGHT = 28.0f;
const float PLAYER_START_X = 100.0f;
const float PLAYER_START_Y = 100.0f;
const float JUMPBOX_HEIGHT = 2.0f;
const float DETECTOR_WIDTH = 8.0f;
const float DETECTOR_HEIGHT = 24.0f;
const float WALL_SLIDE_DETECTOR_OFFSET = 20.0f;
const float DETECTOR_BUFFER = (PLAYER_HEIGHT - DETECTOR_HEIGHT) / 2;
const int JUMP_BUFFER_TIME = 12;
const int ANCHOR_BUFFER = 50;
const int num_sprites{220};

struct PlayerStats {
    
    int health{};
    int max_health{};
    int orbs{};
    int max_orbs{};
    
};

struct PlayerInventoryStats {
    
    uint16_t gem_spinel{};
    uint16_t gem_topaz{};
    uint16_t gem_tourmaline{};
    uint16_t gem_peridot{};
    
    uint16_t flower_lavender{};
    uint16_t flower_daffodil{};
    uint16_t flower_hibiscus{};
    uint16_t flower_orchid{};
    
};


struct PhysicsStats {
    
    float PLAYER_MAX_XVEL = 2.380f;
    float PLAYER_MAX_YVEL = 0.426f;
    
    float AIR_MULTIPLIER = 3.6f;
    
    float PLAYER_GRAV = 0.289f;
    
    float TERMINAL_VELOCITY = 8.0f;

    float PLAYER_VERT_FRIC = 0.712f;
    float PLAYER_HORIZ_FRIC = 0.712;
    float PLAYER_VERT_AIR_FRIC = 0.912f;
    float PLAYER_HORIZ_AIR_FRIC = 0.912f;
    float PLAYER_MASS = 1.0f;

    float X_ACC = 0.794f;
    float Y_ACC = 0.794f;
    
    float JUMP_ACC = 0.8f;
    float JUMP_MAX = 4.387;
    int   JUMP_TIME = 20;
    
    float WALL_SLIDE_THRESHOLD = -1.0f;
    float WALL_SLIDE_SPEED = 1.31f;
    
    float LANDED_THRESHOLD = 3.0f;
    
};

struct SoundboardFlags {
    bool jump{};
    bool step{};
    bool land{};
    bool weapon_swap{};
};

class Player {
public:
    
    using Time = std::chrono::duration<float>;
    
    Player();
    
    //member functions
    void handle_events(sf::Event& event);
    void update(Time dt);
    void render(sf::RenderWindow& win, sf::Vector2<float>& campos);
    void assign_texture(sf::Texture& tex);
    void update_animation();
    
    void sync_components();
    void update_behavior();
    void set_position(sf::Vector2<float> new_pos);
    void update_direction();
    void update_weapon_direction();
    
    //collision
    void handle_map_collision(const Shape& cell, bool is_ramp);
    
    //firing
    sf::Vector2<float> get_fire_point();
    
    //sound
    void play_sounds();
    
    //for debug mode
    std::string print_direction(bool lr);
    
    
    //member vars
    Shape hurtbox{};
    Shape predictive_hurtbox{};
    Shape jumpbox{};
    Shape left_detector{};
    Shape right_detector{};
    Shape wall_slide_detector{};
    
    components::PhysicsComponent physics{};
    components::PlayerBehaviorComponent behavior{};
    behavior::DIR last_dir{};
    PhysicsStats stats{};
    arms::Arsenal loadout{};
    std::vector<arms::WEAPON_TYPE> weapons_hotbar{};
    int current_weapon{};
    
    sf::Vector2<float> apparent_position{};
    sf::Vector2<float> anchor_point{};
    sf::Vector2<float> hand_position{};
    
    PlayerStats player_stats{3, 3, 0, 100};
    PlayerInventoryStats player_inv_stats{0, 0, 0, 0, 0, 0, 0, 0};
    
    //sprites
    sf::Sprite sprite{};
    
    SoundboardFlags soundboard_flags{};
    
    bool move_left{};
    bool move_right{};
    bool look_up{};
    bool look_down{};
    
    bool grav = true;
    bool just_collided = false;
    bool is_colliding_with_level{};
    bool has_left_collision{};
    bool has_right_collision{};
    bool grounded = false;
    bool jump_hold = false;
    bool jump_trigger = false;
    bool can_jump{};
    bool just_jumped{};
    bool is_jump_pressed{};
    bool stopping{};
    bool just_landed{};
    bool left_released{};
    bool right_released{};
    bool is_wall_sliding{};
    int jump_request{};
    
    bool suspended_trigger{};
    bool fall_trigger{};
    bool landed_trigger{};
    bool wall_slide_trigger{};
    bool release_wallslide{};
    bool entered_freefall{};
    bool freefalling{};
    bool sprite_flip{};
    
    bool weapon_fired{};
    bool start_cooldown{};
    
    bool is_any_jump_colllision = false;
    bool is_any_colllision = false;
    int left_aabb_counter = 0;
    int right_aabb_counter = 0;
    int wall_slide_ctr{0};
    
    int jump_height_counter{};
    
};
 /* Player_hpp */
