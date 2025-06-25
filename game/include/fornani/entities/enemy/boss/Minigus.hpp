#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/npc/NPC.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/Shockwave.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include "fornani/gui/StatusBar.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/particle/Sparkler.hpp"
#define MINIGUS_BIND(f) std::bind(&Minigus::f, this)

namespace fornani::enemy {

enum class MinigusState { idle, turn, run, shoot, jump_shoot, hurt, jump, jumpsquat, reload, punch, uppercut, build_invincibility, laugh, snap, rush, struggle, exit, drink, throw_can };
enum class MinigusFlags { recently_hurt, distant_range_activated, battle_mode, theme_song, exit_scene, over_and_out, goodbye, threw_can, punched, soda_pop, second_phase };
enum class MinigunState { deactivated, neutral, charging, firing };
enum class MinigunFlags { exhausted, charging };

class Minigus : public Enemy, public npc::NPC {

  public:
	Minigus(automa::ServiceProvider& svc, world::Map& map);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }
	[[nodiscard]] auto half_health() const -> bool { return health.get_hp() < health.get_max() * 0.5f; }

	fsm::StateFunction state_function = std::bind(&Minigus::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_jump_shoot();
	fsm::StateFunction update_reload();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_punch();
	fsm::StateFunction update_uppercut();
	fsm::StateFunction update_build_invincibility();
	fsm::StateFunction update_laugh();
	fsm::StateFunction update_snap();
	fsm::StateFunction update_rush();
	fsm::StateFunction update_struggle();
	fsm::StateFunction update_exit();
	fsm::StateFunction update_drink();
	fsm::StateFunction update_throw_can();

  private:
	bool anim_debug{};
	bool console_complete{};
	MinigusState state{};
	util::BitFlags<MinigusFlags> status{};
	gui::StatusBar health_bar;
	graphics::SpriteHistory sprite_history{8};

	Direction sprite_direction{};
	Direction pre_direction{};
	Direction post_direction{};
	Direction movement_direction{};

	shape::Shape distant_range{};

	struct {
		sf::Sprite sprite;
		anim::Animation animation{};
		MinigunState state{};
		anim::Parameters neutral{21, 4, 28, -1};
		anim::Parameters deactivated{11, 2, 48, -1};
		anim::Parameters charging{0, 11, 38, 0};
		anim::Parameters firing{13, 8, 10, 1};
		sf::Vector2<float> offset{0, 64};
		sf::Vector2<int> dimensions{78, 30};
		Direction direction;
		util::BitFlags<MinigunFlags> flags{};
	} minigun;

	struct {
		entity::Attack punch{};
		entity::Attack uppercut{};
		entity::Attack rush{};
		entity::Shockwave left_shockwave{{-0.6f, 0.f}};
		entity::Shockwave right_shockwave{{0.6f, 0.f}};
	} attacks{};

	float fire_chance{2.f};
	float snap_chance{10.f};
	float rush_chance{20.f};
	float rush_speed{8.f};
	int health_bar_size{600};

	// packages
	entity::WeaponPackage gun;
	entity::WeaponPackage soda;
	entity::Caution caution{};

	struct {
		util::Cooldown jump{20};
		util::Cooldown rush{400};
		util::Cooldown firing{1000};
		util::Cooldown post_charge{600};
		util::Cooldown post_punch{400};
		util::Cooldown hurt{320};
		util::Cooldown player_punch{80};
		util::Cooldown pre_jump{380};
		util::Cooldown vulnerability{2000};
		util::Cooldown exit{500};
		util::Cooldown struggle{400};
	} cooldowns{};

	vfx::Sparkler sparkler{};

	struct {
		util::Counter snap{};
		util::Counter invincible_turn{};
	} counters{};

	struct {
		sf::Sound jump;
		sf::Sound land;
		sf::Sound crash;
		sf::Sound step;
		sf::Sound punch;
		sf::Sound snap;
		sf::Sound lose_inv;
		sf::Sound charge;
		sf::Sound build;
		sf::Sound inv;
		sf::Sound soda;
	} sounds;

	struct {
		sf::Sound hurt_1;
		sf::Sound hurt_2;
		sf::Sound hurt_3;
		sf::Sound laugh_1;
		sf::Sound laugh_2;
		sf::Sound grunt;
		sf::Sound aww;
		sf::Sound babyimhome;
		sf::Sound deepspeak;
		sf::Sound doge;
		sf::Sound dontlookatme;
		sf::Sound exhale;
		sf::Sound getit;
		sf::Sound greatidea;
		sf::Sound itsagreatday;
		sf::Sound long_death;
		sf::Sound long_moan;
		sf::Sound momma;
		sf::Sound mother;
		sf::Sound ok_1;
		sf::Sound ok_2;
		sf::Sound pizza;
		sf::Sound poh;
		sf::Sound quick_breath;
		sf::Sound thatisverysneeze; // dash cue
		sf::Sound whatisit;
		sf::Sound woob; // jumping
	} voice;

	util::Cycle hurt_color{2};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 48, -1};
	anim::Parameters shoot{10, 1, 38, -1};
	anim::Parameters jumpsquat{18, 1, 58, 0};
	anim::Parameters hurt{21, 4, 24, 2};
	anim::Parameters jump{14, 1, 22, -1};
	anim::Parameters jump_shoot{32, 1, 42, -1};
	anim::Parameters reload{7, 7, 18, 0};
	anim::Parameters turn{18, 2, 42, 0};
	anim::Parameters run{14, 4, 42, 2};
	anim::Parameters punch{28, 4, 32, 0};
	anim::Parameters uppercut{35, 4, 32, 0};
	anim::Parameters struggle{35, 1, 24, -1};
	anim::Parameters build_invincibility{33, 2, 22, 4};
	anim::Parameters laugh{25, 3, 24, 4};
	anim::Parameters snap{39, 3, 42, 0};
	anim::Parameters rush{66, 4, 22, -1};

	anim::Parameters drink{42, 16, 20, 0};
	anim::Parameters throw_can{58, 8, 22, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(MinigusState next, anim::Parameters params);

	io::Logger m_logger{"boss"};
};

} // namespace fornani::enemy
