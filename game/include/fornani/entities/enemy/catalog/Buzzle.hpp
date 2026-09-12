
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>

#define BUZZLE_BIND(f) std::bind(&Buzzle::f, this)

namespace fornani::enemy {

enum class BuzzleState : std::uint8_t { idle, turn, signal, divebomb };

class Buzzle final : public Enemy, public StateMachine<BuzzleState> {
  public:
	Buzzle(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Buzzle::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_signal();
	fsm::StateFunction update_divebomb();

  private:
	components::SteeringBehavior m_steering{};
	util::Cooldown m_dive_timer;
	util::Cooldown m_recovery;
	sf::Vector2f m_target{};

	bool change_state(BuzzleState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
