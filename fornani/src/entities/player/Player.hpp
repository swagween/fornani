
#pragma once

#include <array>
#include <memory>
#include "../../components/BehaviorComponent.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../particle/Attractor.hpp"
#include "../../utils/BitFlags.hpp"
#include "../../utils/Collider.hpp"
#include "../../weapon/Arsenal.hpp"
#include "PlayerController.hpp"

namespace player {

float const PLAYER_WIDTH = 18.0f;
float const PLAYER_HEIGHT = 24.0f;
float const head_height{8.f};
float const PLAYER_START_X = 100.0f;
float const PLAYER_START_Y = 100.0f;
float const JUMPBOX_HEIGHT = 8.0f;
float const DETECTOR_WIDTH = 8.0f;
float const DETECTOR_HEIGHT = 22.0f;
float const WALL_SLIDE_DETECTOR_OFFSET = 20.0f;
float const DETECTOR_BUFFER = (PLAYER_HEIGHT - DETECTOR_HEIGHT) / 2;
int const JUMP_BUFFER_TIME = 12;
int const INVINCIBILITY_TIME = 200;
int const ANCHOR_BUFFER = 50;
int const num_sprites{220};
float const stopped_threshold{0.2f};

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
	float grav{};
	float ground_fric{};
	float air_fric{};
	float x_acc{};
	float air_multiplier{};
	float jump_velocity{};
	float jump_release_multiplier{};
	float hurt_acc{};
	sf::Vector2<float> maximum_velocity{};
	float mass{};
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
	hold,		 // true if jump is pressed and permanently false once released, until player touches the ground again
	trigger,	 // true for one frame if jump is pressed and the player is grounded
	can_jump,	 // true if the player is grounded
	just_jumped, // used for updating animation
	is_pressed,	 // true if the jump button is pressed, false if not. independent of player's state.
	is_released, // true if jump released midair, reset upon landing
	jumping,	 // true if jumpsquat is over, falce once player lands
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

enum class Input { restricted, no_anim, exit_request, inspecting, inspecting_trigger };

enum class State { alive };

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

	// init (violates RAII but must happen after resource path is set)
	void init();
	// member functions
	void handle_events(sf::Event& event);
	void update(Time dt);
	void render(sf::RenderWindow& win, sf::Vector2<float>& campos);
	void assign_texture(sf::Texture& tex);
	void update_animation();
	void update_sprite();
	void flash_sprite();
	void calculate_sprite_offset();

	void update_behavior();
	void set_position(sf::Vector2<float> new_pos);
	void update_direction();
	void update_weapon();
	void walk();
	void autonomous_walk();
	void hurt(int amount);
	void update_antennae();

	void restrict_inputs();
	void unrestrict_inputs();
	void restrict_animation();
	void no_move();

	bool grounded() const;
	bool moving();
	bool moving_at_all();

	// firing
	sf::Vector2<float> get_fire_point();

	// level events
	void make_invincible();
	void update_invincibility();
	bool is_invincible() const;
	void kill();
	void start_over();

	void reset_flags();
	void total_reset();

	// map helpers
	behavior::DIR_LR entered_from();

	// sound
	void play_sounds();

	// for debug mode
	std::string print_direction(bool lr);

	//components
	controllers::PlayerController controller{};
	shape::Collider collider{{PLAYER_WIDTH, PLAYER_HEIGHT}, {PLAYER_START_X, PLAYER_START_Y}};
	shape::Collider head{};
	components::PlayerBehaviorComponent behavior{};
	behavior::DIR last_dir{};
	arms::Arsenal loadout{};
	std::vector<arms::WEAPON_TYPE> weapons_hotbar{};
	int current_weapon{};

	sf::Vector2<float> apparent_position{};
	sf::Vector2<float> anchor_point{};
	sf::Vector2<float> hand_position{};
	sf::Vector2<float> sprite_offset{};
	sf::Vector2<float> sprite_dimensions{};
	sf::Vector2<float> sprite_position{};

	std::vector<vfx::Attractor> antennae{};
	sf::Vector2<float> antenna_offset{4.f, -13.f};

	PlayerStats player_stats{3, 3, 0, 99999};
	PlayerInventoryStats player_inv_stats{0, 0, 0, 0, 0, 0, 0, 0};
	PhysicsStats physics_stats{};
	PlayerFlags flags{};

	Counters counters{};

	// fixed animation time step variables
	Time dt{0.001f};
	Clock::time_point current_time = Clock::now();
	Time accumulator{0.0f};

	// sprites
	sf::Sprite sprite{};

	bool grav = true;

	int jump_request{};

	bool just_hurt{};
	bool weapon_fired{};
	bool start_cooldown{};
	bool sprite_flip{};

	int wall_slide_ctr{0};
};

} // namespace player
