
#pragma once

#include <fornani/entities/enemy/Boss.hpp>
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#include "fornani/entities/packages/Shockwave.hpp"
#include "fornani/graphics/SpriteHistory.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/particle/Sparkler.hpp"

#define MINIGUS_BIND(f) std::bind(&Minigus::f, this)

namespace fornani::enemy {

enum class MinigusMode : std::uint8_t { neutral, battle_one, battle_two, last_words, exit };
enum class MinigusState : std::uint8_t { idle, turn, run, shoot, jump_shoot, jumpsquat, airborne, land, grab_soda, drink, throw_can, stagger, struggle, recover, laugh, punch, reload, snap, wind_up, charge, exit };
enum class MinigusFlags : std::uint8_t { jump_shoot };
enum class MinigunState : std::uint8_t { deactivated, neutral, charging, firing };
enum class MinigunFlags : std::uint8_t { exhausted, charging };

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

class Minigus : public Boss, public StateMachine<MinigusState> {

  public:
	Minigus(automa::ServiceProvider& svc, world::Map& map, SceneContext& context);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	[[nodiscard]] auto invincible() const -> bool { return !flags.state.test(StateFlags::vulnerable); }

	void request(MinigusState const state) { StateMachine<MinigusState>::request(state); }
	void set_state(MinigusState const to) { StateMachine<MinigusState>::p_state.actual = to; }

	fsm::StateFunction state_function = std::bind(&Minigus::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_jump_shoot();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_airborne();
	fsm::StateFunction update_land();
	fsm::StateFunction update_grab_soda();
	fsm::StateFunction update_drink();
	fsm::StateFunction update_throw_can();
	fsm::StateFunction update_stagger();
	fsm::StateFunction update_struggle();
	fsm::StateFunction update_recover();
	fsm::StateFunction update_laugh();
	fsm::StateFunction update_punch();
	fsm::StateFunction update_snap();
	fsm::StateFunction update_reload();
	fsm::StateFunction update_wind_up();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_exit();

  private:
	[[nodiscard]] auto is(MinigusState const test) const -> bool { return StateMachine<MinigusState>::is_state(test); }
	[[nodiscard]] auto is_battle_mode() const -> bool { return (m_mode == MinigusMode::battle_one || m_mode == MinigusMode::battle_two) && console_complete; }

	void shoot();

	bool change_state(MinigusState next, anim::Parameters params);

	void debug();

  private:
	bool anim_debug{};
	bool console_complete{};
	bool init{true};
	MinigusMode m_mode{};
	util::BitFlags<MinigusFlags> m_flags{};
	graphics::SpriteHistory sprite_history{8};

	Direction sprite_direction{};
	Direction movement_direction{};

	shape::Shape distant_range{};

	Minigun m_minigun;

	struct {
		entity::Attack punch{};
		entity::Attack rush{};
		std::vector<entity::Shockwave> shockwaves;
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

	util::Cycle m_barrel_cycle{6};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	SceneContext* m_context;

	io::Logger m_logger{"boss"};
};

} // namespace fornani::enemy
