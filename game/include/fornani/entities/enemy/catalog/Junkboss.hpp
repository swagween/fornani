
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define JUNKBOSS_BIND(f) std::bind(&Junkboss::f, this)

namespace fornani::enemy {

enum class JunkbossState : std::uint8_t { idle, turn, toss };
enum class JunkbossVariant : std::uint8_t { grenade };
enum class JunkbossFlags : std::uint8_t { toss };

class Junkboss final : public Enemy, public StateMachine<JunkbossState>, public Flaggable<JunkbossFlags> {
  public:
	Junkboss(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Junkboss::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_toss();

  private:
	JunkbossVariant m_variant{};

	components::SteeringBehavior m_steering{};
	util::Cooldown m_toss_time;
	util::Cooldown m_switch_sides;
	std::optional<entity::WeaponPackage> m_bomb{};

	bool change_state(JunkbossState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
