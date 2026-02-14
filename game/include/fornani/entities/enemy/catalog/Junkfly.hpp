
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Caution.hpp>
#include <fornani/utils/Flaggable.hpp>

#define JUNKFLY_BIND(f) std::bind(&Junkfly::f, this)

namespace fornani::enemy {

enum class JunkflyState { idle, turn, toss };
enum class JunkflyVariant { grenade };
enum class JunkflyFlags { toss };

class Junkfly final : public Enemy, public StateMachine<JunkflyState>, public Flaggable<JunkflyFlags> {
  public:
	Junkfly(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	void debug();

	fsm::StateFunction state_function = std::bind(&Junkfly::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_toss();

  private:
	JunkflyVariant m_variant{};

	components::SteeringBehavior m_steering{};
	util::Cooldown m_toss_time;
	std::optional<entity::WeaponPackage> m_bomb{};

	bool change_state(JunkflyState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
