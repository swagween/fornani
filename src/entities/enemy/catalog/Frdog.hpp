#pragma once

#include "../Enemy.hpp"
#define BIND(f) std::bind(&Frdog::f, this)

namespace enemy {

class Frdog : public Enemy {

  public:
	Frdog() = default;
	Frdog(automa::ServiceProvider& svc);
	void unique_update(automa::ServiceProvider& svc, world::Map& map) override;

	fsm::StateFunction state_function = std::bind(&Frdog::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_sit();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_run();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_bark();
};

} // namespace enemy