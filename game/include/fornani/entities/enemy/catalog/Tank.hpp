#pragma once

#include <fornani/audio/Soundboard.hpp>
#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Caution.hpp"
#define TANK_BIND(f) std::bind(&Tank::f, this)

namespace fornani::enemy {

enum class TankState { idle, run, shoot_horizontal, shoot_vertical, jumpsquat, jump, land, turn, type, alert, pocket, sleep, drink };
enum class TankFlags { show_weapon, shorthop };
enum class TankVariant { watchman, typist };

class Tank final : public Enemy {

  public:
	Tank(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Tank::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_shoot_horizontal();
	fsm::StateFunction update_shoot_vertical();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_land();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_type();
	fsm::StateFunction update_alert();
	fsm::StateFunction update_pocket();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_drink();

	[[nodiscard]] auto is_mid_run() { return m_cooldowns.run.is_almost_complete(); }
	[[nodiscard]] auto has_been_alerted() { return m_cooldowns.alerted.running(); }

  private:
	void request(TankState to) { m_state.desired = to; }

	struct {
		audio::Tank hurt{};
	} m_sounds{};

	TankVariant m_variant{};
	util::BitFlags<TankFlags> m_flags{};
	struct {
		TankState actual{};
		TankState desired{};
	} m_state{};

	struct {
		util::Cooldown alerted{3200};
		util::Cooldown post_jump{200};
		util::Cooldown run{80};
	} m_cooldowns{};

	float fire_chance{50.f};

	// packages
	entity::FloatingPart m_gun;
	entity::WeaponPackage m_weapon;
	entity::Caution m_caution{};

	// extra detectors
	shape::Shape m_vertical_range{};
	shape::Shape m_shoulders{};
	shape::Shape m_lower_range{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 28, -1};
	anim::Parameters run{6, 4, 38, 2};
	anim::Parameters shoot_horizontal{10, 4, 22, 0};
	anim::Parameters shoot_vertical{14, 4, 22, 0};
	anim::Parameters jumpsquat{18, 5, 22, 0, true};
	anim::Parameters jump{23, 4, 22, 0, true};
	anim::Parameters land{27, 3, 22, 0};
	anim::Parameters turn{30, 2, 32, 0};
	anim::Parameters type{32, 2, 128, -1};
	anim::Parameters alert{34, 7, 32, 0};
	anim::Parameters pocket{41, 6, 32, 0};
	anim::Parameters sleep{47, 2, 256, -1};
	anim::Parameters drink{49, 6, 32, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(TankState next, anim::Parameters params);
};

} // namespace fornani::enemy
