
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

enum class MinigusState : std::uint8_t {
	idle,
	turn,
	run,
	shoot,
	jump_shoot,
	jumpsquat,
	airborne,
	land,
	grab_soda,
	drink,
	throw_can,
	stagger,
	struggle,
	recover,
	laugh,
	punch,
	reload,
	snap,
	wind_up,
	charge,
	long_charge,
	charge_turn,
	bump,
	exit
};
enum class MinigusFlags : std::uint8_t { aerial_shoot, player_is_punchable, defeated, dropped_loot, show_minigun, short_hop };

class Minigun : public Animatable {
  public:
	Minigun(automa::ServiceProvider& svc) : Animatable(svc, "minigus_minigun", {57, 22}) {}
	void update(sf::Vector2f const target);
	void render(sf::Vector2f const cam);
	void set_physics_position(sf::Vector2f const to) { m_physics.position = to; }

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
	fsm::StateFunction update_long_charge();
	fsm::StateFunction update_charge_turn();
	fsm::StateFunction update_exit();
	fsm::StateFunction update_bump();

  private:
	[[nodiscard]] auto is(MinigusState const test) const -> bool { return StateMachine<MinigusState>::is_state(test); }
	[[nodiscard]] auto has_side_collision() const -> bool;
	[[nodiscard]] auto sidestep() const -> bool { return cooldowns.above.get_normalized() < 0.1f; }

	void shoot();

	bool change_state(MinigusState next, anim::Parameters params);

	void debug();

  private:
	bool anim_debug{};
	bool console_complete{};
	bool init{true};
	util::BitFlags<MinigusFlags> m_flags{};
	graphics::SpriteHistory sprite_history{8};

	Direction sprite_direction{};
	Direction movement_direction{};

	shape::Shape distant_range{};
	shape::Shape jump_range;

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
		util::Cooldown post_shot{1600};
		util::Cooldown aerial_shoot{60};
		util::Cooldown long_charge{1300};
		util::Cooldown above{1000};
	} cooldowns{};

	vfx::Sparkler sparkler;
	Animatable m_tattoo;

	struct {
		util::Counter snap{};
		util::Counter invincible_turn{};
	} counters{};

	util::Cycle m_barrel_cycle{6};

	sf::Vector2f m_gun_offset{};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	SceneContext* m_context;

	io::Logger m_logger{"boss"};
};

} // namespace fornani::enemy
