#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define BROVLE_BIND(f) std::bind(&Brovle::f, this)

namespace fornani::enemy {

enum class BrovleState : std::uint8_t { idle, sweep, run, jumpsquat, jump, land, slash, turn };
enum class BrovleVariant : std::uint8_t { swordfighter };
enum class BrovleFlags : std::uint8_t { step_detected, projectile };

class Brovle final : public Enemy, public StateMachine<BrovleState>, public Flaggable<BrovleFlags> {
  public:
	Brovle(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Brovle::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_jump();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_slash();
	fsm::StateFunction update_sweep();
	fsm::StateFunction update_land();

  private:
	BrovleVariant m_variant{};

	util::Cooldown m_jump_time;
	util::Cooldown m_switch_sides;
	std::optional<entity::WeaponPackage> m_sword_wave{};
	entity::Caution m_caution{};

	bool change_state(BrovleState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
