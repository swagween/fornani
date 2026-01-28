
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Attack.hpp>
#include <fornani/entities/packages/Caution.hpp>

#define MASTIFF_BIND(f) std::bind(&Mastiff::f, this)

namespace fornani::enemy {

enum class MastiffState { idle, run, bite, turn };

class Mastiff final : public Enemy, public StateMachine<MastiffState> {
  public:
	Mastiff(automa::ServiceProvider& svc, world::Map& map, int variant);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;

	fsm::StateFunction state_function = std::bind(&Mastiff::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_run();
	fsm::StateFunction update_bite();
	fsm::StateFunction update_turn();

  private:
	entity::Caution m_caution{};
	entity::Attack m_bite;
	bool change_state(MastiffState next, anim::Parameters params);

	automa::ServiceProvider* m_services;
};

} // namespace fornani::enemy
