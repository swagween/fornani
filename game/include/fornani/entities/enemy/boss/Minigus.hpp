#pragma once

#include <capo/engine.hpp>
#include <fornani/entity/NPC.hpp>
#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/Shockwave.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include "fornani/gui/BossHealth.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/particle/Sparkler.hpp"

#define MINIGUS_BIND(f) std::bind(&Minigus::f, this)

namespace fornani::enemy {

enum class MinigusMode { neutral, battle_one, battle_two, last_words, exit };
enum class MinigusState { idle, turn, run, shoot, jump_shoot, hurt, jump, jumpsquat, reload, punch, uppercut, build_invincibility, laugh, snap, rush, struggle, exit, drink, throw_can };
enum class MinigusFlags { recently_hurt, distant_range_activated, battle_mode, theme_song, exit_scene, over_and_out, goodbye, threw_can, punched, soda_pop, second_phase };
enum class MinigunState { deactivated, neutral, charging, firing };
enum class MinigunFlags { exhausted, charging };

class Minigun : public Animatable {
  public:
	Minigun(automa::ServiceProvider& svc) : Animatable(svc, "minigus_minigun", {39, 15}) {}
	void update(sf::Vector2f const target);
	void render(sf::Vector2f const cam);
	void set_physics_position(sf::Vector2f const to) { m_physics.position = to; }
	MinigunState state{};
	sf::Vector2f offset{};
	anim::Parameters neutral{21, 4, 28, -1};
	anim::Parameters deactivated{11, 2, 48, -1};
	anim::Parameters charging{0, 11, 38, 0};
	anim::Parameters firing{13, 8, 10, 1};
	util::BitFlags<MinigunFlags> flags{};

  private:
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
};

class Minigus : public Enemy, public NPC {

  public:
	Minigus(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
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
	void request(MinigusState to) { m_state.desired = to; }
	[[nodiscard]] auto is(MinigusState const test) const -> bool { return m_state.actual == test; }
	[[nodiscard]] auto is_battle_mode() const -> bool { return (m_mode == MinigusMode::battle_one || m_mode == MinigusMode::battle_two) && !m_console->has_value(); }

	struct {
		MinigusState actual{};
		MinigusState desired{};
	} m_state{};

	bool anim_debug{};
	bool console_complete{};
	bool init{true};
	MinigusMode m_mode{};
	util::BitFlags<MinigusFlags> status{};
	gui::BossHealth health_bar;
	graphics::SpriteHistory sprite_history{8};

	Direction sprite_direction{};
	Direction movement_direction{};

	shape::Shape distant_range{};

	Minigun m_minigun;

	struct {
		entity::Attack punch{};
		entity::Attack uppercut{};
		entity::Attack rush{};
		entity::Shockwave left_shockwave;
		entity::Shockwave right_shockwave;
	} attacks{};

	float fire_chance{2.f};
	float snap_chance{10.f};
	float rush_chance{20.f};
	float rush_speed{8.f};

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

	vfx::Sparkler sparkler;

	struct {
		util::Counter snap{};
		util::Counter invincible_turn{};
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

	std::optional<std::unique_ptr<gui::Console>>* m_console;

	io::Logger m_logger{"boss"};
};

} // namespace fornani::enemy
