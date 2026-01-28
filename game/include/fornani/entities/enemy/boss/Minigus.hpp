
#pragma once

#include <capo/engine.hpp>
#include <fornani/entities/enemy/Boss.hpp>
#include <fornani/entity/NPC.hpp>
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

class Minigus : public Boss, public NPC, public StateMachine<MinigusState> {

  public:
	Minigus(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }

	void request(MinigusState const state) { StateMachine<MinigusState>::request(state); }
	void set_state(MinigusState const to) { StateMachine<MinigusState>::p_state.actual = to; }

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
	[[nodiscard]] auto is(MinigusState const test) const -> bool { return StateMachine<MinigusState>::is_state(test); }
	[[nodiscard]] auto is_battle_mode() const -> bool { return (m_mode == MinigusMode::battle_one || m_mode == MinigusMode::battle_two) && !m_console->has_value(); }

	bool anim_debug{};
	bool console_complete{};
	bool init{true};
	MinigusMode m_mode{};
	util::BitFlags<MinigusFlags> status{};
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
		util::Cooldown hurt_sound{32};
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

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(MinigusState next, anim::Parameters params);

	std::optional<std::unique_ptr<gui::Console>>* m_console;

	io::Logger m_logger{"boss"};
};

} // namespace fornani::enemy
