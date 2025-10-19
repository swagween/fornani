
#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Attack.hpp"
#include "fornani/entities/packages/Caution.hpp"
#define HULMET_BIND(f) std::bind(&Hulmet::f, this)

namespace fornani::enemy {

enum class HulmetState : std::uint8_t { idle, turn, run, jump, alert, sleep, shoot, roll, panic, reload };
enum class HulmetVariant : std::uint8_t { gunner, bodyguard };
enum class HulmetFlags : std::uint8_t { out_of_ammo };

class Hulmet final : public Enemy, public StateMachine<HulmetState> {
  public:
	Hulmet(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Hulmet::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_alert();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_roll();
	fsm::StateFunction update_panic();
	fsm::StateFunction update_reload();

	[[nodiscard]] auto is_mid_run() { return m_cooldowns.run.is_almost_complete(); }
	[[nodiscard]] auto was_alerted() { return m_cooldowns.alerted.running(); }
	[[nodiscard]] auto is_out_of_ammo() { return m_flags.test(HulmetFlags::out_of_ammo); }

  private:
	HulmetVariant m_variant{};

	// packages
	struct {
		entity::FloatingPart gun;
	} m_parts;

	entity::Caution m_caution{};
	entity::WeaponPackage m_weapon;

	struct {
		util::Cooldown post_fire{400};
		util::Cooldown alerted{1800};
		util::Cooldown post_jump{200};
		util::Cooldown post_roll{200};
		util::Cooldown run{80};
	} m_cooldowns{};

	// lookup, duration, framerate, num_loops
	struct {
		anim::Parameters idle{0, 6, 32, -1};
		anim::Parameters turn{26, 2, 32, 0};
		anim::Parameters run{5, 4, 28, 4};
		anim::Parameters jump{9, 1, 64, 0};
		anim::Parameters roll{10, 4, 24, 2};
		anim::Parameters sleep{14, 6, 64, 3};
		anim::Parameters alert{20, 6, 24, 0};
		anim::Parameters shoot{28, 3, 24, 1};
		anim::Parameters panic{31, 7, 24, 0};
		anim::Parameters reload{38, 5, 64, 0};
	} m_animations{};

	util::BitFlags<HulmetFlags> m_flags{};

	float m_jump_force;
	int m_jump_time{4};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(HulmetState next, anim::Parameters params);
};

} // namespace fornani::enemy
