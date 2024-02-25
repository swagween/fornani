
#pragma once

#include <iostream>
#include "../../utils/StateFunction.hpp"
#include "../behavior/Animation.hpp"
#define BIND(f) std::bind(&PlayerAnimation::f, this)

namespace player {

inline anim::Parameters idle{20, 8, 14, false};

class PlayerAnimation {

  public:
	PlayerAnimation() {
		state_function = state_function();
		animation.start();
	}

	anim::Animation animation{};

	void update();
	int get_frame() const;

	fsm::StateFunction state_function = std::bind(&PlayerAnimation::update_idle, this);

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_rise();
	fsm::StateFunction update_suspend();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_land();
};

} // namespace critter
