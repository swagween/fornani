
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/animation/StateMachine.hpp>
#include <fornani/utils/StateFunction.hpp>
#define CHAMPIONJ5_BIND(f) std::bind(&ChampionJ5::f, this)

namespace fornani {

enum class ChampionJ5State { flying, land, grounded, take_off };

class ChampionJ5 : public Mobile, public StateMachine<ChampionJ5State> {
  public:
	ChampionJ5(automa::ServiceProvider& svc);
	void update();
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_target(sf::Vector2f to) { m_target = to; }

	fsm::StateFunction state_function = std::bind(&ChampionJ5::update_flying, this);
	fsm::StateFunction update_flying();
	fsm::StateFunction update_land();
	fsm::StateFunction update_grounded();
	fsm::StateFunction update_take_off();

  private:
	bool change_state(ChampionJ5State next, anim::Parameters params);

  private:
	Animatable m_propeller;
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_target{};
};

} // namespace fornani
