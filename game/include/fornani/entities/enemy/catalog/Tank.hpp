#pragma once

#include <fornani/audio/Soundboard.hpp>
#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>
#define TANK_BIND(f) std::bind(&Tank::f, this)

namespace fornani::enemy {

enum class TankState { idle, run, shoot_horizontal, shoot_vertical, jumpsquat, jump, land, turn, type, alert, pocket, sleep, drink };
enum class TankFlags { show_weapon, shorthop };
enum class TankVariant { watchman, typist };
enum class TankMode { neutral, hostile };

class Tank final : public Enemy, public StateMachine<TankState>, public Flaggable<TankFlags> {

  public:
	Tank(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	void gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Tank::update_type, this);
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
	struct {
		audio::Tank hurt{};
	} m_sounds{};

	TankVariant m_variant{};
	TankMode m_mode{};

	struct {
		util::Cooldown alerted{2000};
		util::Cooldown post_jump{400};
		util::Cooldown run{80};
		util::Cooldown post_shoot{280};
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

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(TankState next, anim::Parameters params);
	void debug();
	bool m_debug;
};

} // namespace fornani::enemy
