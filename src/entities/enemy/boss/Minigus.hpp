#pragma once

#include "../../npc/NPC.hpp"
#include "../Enemy.hpp"
#include "../../../components/CircleSensor.hpp"
#include "../../../gui/StatusBar.hpp"
#define MINIGUS_BIND(f) std::bind(&Minigus::f, this)

namespace enemy {

enum class MinigusState { idle, turn, run, shoot, jump_shoot, hurt, jump, jumpsquat, reload, punch, uppercut, build_invincibility, laugh, snap, rush };
enum class MinigusFlags { recently_hurt, distant_range_activated };
enum class MinigunState { deactivated, neutral, charging, firing };
enum class MinigunFlags { exhausted, charging };

class Minigus : public Enemy, public npc::NPC {

  public:
	Minigus() = default;
	Minigus(automa::ServiceProvider& svc, world::Map& map, gui::Console& console);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) override;

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

  private:
	bool anim_debug{};
	MinigusState state{};
	util::BitFlags<MinigusFlags> status{};
	gui::StatusBar health_bar;

	dir::Direction pre_direction{};
	dir::Direction post_direction{};
	dir::Direction movement_direction{};

	shape::Shape distant_range{};

	struct {
		sf::Sprite sprite{};
		anim::Animation animation{};
		MinigunState state{};
		anim::Parameters neutral{21, 4, 28, -1};
		anim::Parameters deactivated{11, 2, 48, -1};
		anim::Parameters charging{0, 11, 38, 0};
		anim::Parameters firing{13, 8, 12, 1};
		sf::Vector2<float> offset{0, 64};
		sf::Vector2<int> dimensions{78, 30};
		dir::Direction direction;
		util::BitFlags<MinigunFlags> flags{};
	} minigun{};

	struct {
		shape::Collider head{};
	} colliders{};

	struct {
		entity::Attack punch{};
		entity::Attack uppercut{};
		entity::Shockwave left_shockwave{{-0.5f, 0.f}};
		entity::Shockwave right_shockwave{{0.5f, 0.f}};
	} attacks{};

	int fire_chance{1};
	int snap_chance{5};
	int rush_chance{8};

	// packages
	entity::WeaponPackage gun;
	entity::Caution caution{};

	struct {
		util::Cooldown jump{20};
		util::Cooldown rush{600};
		util::Cooldown running_time{40};
		util::Cooldown firing{1000};
		util::Cooldown post_charge{600};
		util::Cooldown post_punch{400};
		util::Cooldown hurt{320};
		util::Cooldown player_punch{80};
		util::Cooldown pre_jump{380};
		util::Cooldown vulnerability{2000};
	} cooldowns{};

	struct {
		util::Counter snap{};
	} counters{};

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
	anim::Parameters run{14, 4, 42, -1};
	anim::Parameters punch{28, 4, 32, 0};
	anim::Parameters uppercut{35, 4, 32, 0};
	anim::Parameters build_invincibility{33, 2, 28, 4};
	anim::Parameters laugh{25, 3, 44, 4};
	anim::Parameters snap{39, 3, 42, 0};
	anim::Parameters rush{14, 4, 22, -1};

	automa::ServiceProvider* m_services;
	world::Map* m_map;
	gui::Console* m_console;

	bool change_state(MinigusState next, anim::Parameters params);
};

} // namespace enemy