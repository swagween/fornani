
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

#define MIZZLE_BIND(f) std::bind(&Mizzle::f, this)

namespace fornani::enemy {

enum class MizzleState { idle, turn };

class Mizzle final : public Enemy, public StateMachine<MizzleState> {
  public:
	Mizzle(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Mizzle::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

  private:
	components::SteeringBehavior m_steering{};

	bool change_state(MizzleState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
