//
//  Player.hpp
//  for_loop
//
//  Created by Alex Frasca on 10/12/18.
//  Copyright © 2018 Western Forest Studios. All rights reserved.
//
#pragma once

#include "../../utils/Collider.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../components/BehaviorComponent.hpp"
#include "../../weapon/Arsenal.hpp"
#include "../../utils/BitFlags.hpp"
#include <array>
#include <memory>

const float PLAYER_WIDTH = 24.0f;
const float PLAYER_HEIGHT = 24.0f;
const float PLAYER_START_X = 100.0f;
const float PLAYER_START_Y = 100.0f;
const float JUMPBOX_HEIGHT = 8.0f;
const float DETECTOR_WIDTH = 8.0f;
const float DETECTOR_HEIGHT = 22.0f;
const float WALL_SLIDE_DETECTOR_OFFSET = 20.0f;
const float DETECTOR_BUFFER = (PLAYER_HEIGHT - DETECTOR_HEIGHT) / 2;
const int JUMP_BUFFER_TIME = 12;
const int INVINCIBILITY_TIME = 200;
const int ANCHOR_BUFFER = 50;
const int num_sprites{220};

struct PlayerStats {
    
    int health{};
    int max_health{};
    int orbs{};
    int max_orbs{};
    
};

struct PlayerInventoryStats {
    
    uint32_t gem_spinel{};
    uint32_t gem_topaz{};
    uint32_t gem_tourmaline{};
    uint32_t gem_peridot{};
    
    uint32_t flower_lavender{};
    uint32_t flower_daffodil{};
    uint32_t flower_hibiscus{};
    uint32_t flower_orchid{};
    
};


struct PhysicsStats {

    float PLAYER_MAX_XVEL = 2.380f;
    float PLAYER_MAX_YVEL = 0.426f;

    float AIR_MULTIPLIER = 2.268f;

    float PLAYER_GRAV = 0.003f;

    float TERMINAL_VELOCITY = 2.1f;

    float PLAYER_GROUND_FRIC = 0.979f;
    float PLAYER_HORIZ_AIR_FRIC = 0.990f;
    float PLAYER_VERT_AIR_FRIC = 0.956f;

    float X_ACC = 0.056f;
    float X_ACC_AIR = 0.056f;

    float JUMP_MAX = 0.418f;
    
    float WALL_SLIDE_THRESHOLD = -1.0f;
    float WALL_SLIDE_SPEED = 1.31f;
    float PLAYER_MASS = 1.0f;

    float JUMP_RELEASE_MULTIPLIER = 0.65f;

    float HURT_ACC = 0.15f;
    
};

struct Counters {
    int invincibility{};
};

enum class Soundboard {
    jump,
    step,
    land,
    weapon_swap,
    hurt,
};

enum class Jump {
    hold, //true if jump is pressed and permanently false once released, until player touches the ground again
    trigger, //true for one frame if jump is pressed and the player is grounded
    can_jump, //true if the player is grounded
    just_jumped, //used for updating animation
    is_pressed, //true if the jump button is pressed, false if not. independent of player's state.
    is_released, //true if jump released midair, reset upon landing
    jumping, //true if jumpsquat is over, falce once player lands
};

enum class Movement {

	move_left,
	move_right,
	look_up,
	look_down,
	left_released,
	right_released,

	stopping,
	just_stopped,
	suspended_trigger,
	fall_trigger,
	landed_trigger,
	entered_freefall,
	freefalling,
    autonomous_walk,

	is_wall_sliding,
	wall_slide_trigger,
	release_wallslide,
};

enum class Input {
    restricted,
    no_anim,
    exit_request,
    inspecting,
    inspecting_trigger
};

enum class State {
    alive
};

struct PlayerFlags {
    util::BitFlags<Soundboard> sounds{};
    util::BitFlags<Jump> jump{};
    util::BitFlags<Movement> movement{};
    util::BitFlags<Input> input{};
    util::BitFlags<State> state{};
};

class Player {
public:

    using Clock = std::chrono::steady_clock;
    using Time = std::chrono::duration<float>;
    
    Player();
    
    //member functions
    void handle_events(sf::Event& event);
    void update(Time dt);
    void render(sf::RenderWindow& win, sf::Vector2<float>& campos);
    void assign_texture(sf::Texture& tex);
    void update_animation();
    void update_sprite();
    void flash_sprite();
    
    void update_behavior();
    void set_position(sf::Vector2<float> new_pos);
    void update_direction();
    void update_weapon_direction();
    void walk();
    void autonomous_walk();

    void restrict_inputs();
    void unrestrict_inputs();
    void restrict_animation();
    void no_move();

    bool grounded();
    bool moving();
    
    //firing
    sf::Vector2<float> get_fire_point();

    //level events
    void make_invincible();
    void update_invincibility();
    bool is_invincible();
    void kill();
    void start_over();

    //map helpers
    behavior::DIR_LR entered_from();
    
    //sound
    void play_sounds();
    
    //for debug mode
    std::string print_direction(bool lr);

    shape::Collider collider{ {PLAYER_WIDTH, PLAYER_HEIGHT}, {PLAYER_START_X, PLAYER_START_Y} };
    components::PlayerBehaviorComponent behavior{};
    behavior::DIR last_dir{};
    arms::Arsenal loadout{};
    std::vector<arms::WEAPON_TYPE> weapons_hotbar{};
    int current_weapon{};
    
    sf::Vector2<float> apparent_position{};
    sf::Vector2<float> anchor_point{};
    sf::Vector2<float> hand_position{};
    
    PlayerStats player_stats{3, 3, 0, 100};
    PlayerInventoryStats player_inv_stats{0, 0, 0, 0, 0, 0, 0, 0};
    PhysicsStats stats{};
    PlayerFlags flags{};
    
    Counters counters{};

    //fixed animation time step variables
    Time dt{ 0.001f };
    Clock::time_point current_time = Clock::now();
    Time accumulator{ 0.0f };

    //sprites
    sf::Sprite sprite{};
    
    bool grav = true;

    
    int jump_request{};

    bool just_hurt{};
    
    bool weapon_fired{};
    bool start_cooldown{};

    bool sprite_flip{};

    int wall_slide_ctr{0};
    
};
 /* Player_hpp */
