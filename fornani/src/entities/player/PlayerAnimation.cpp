#include "PlayerAnimation.hpp"

namespace player {

	

void PlayerAnimation::update() {
	if (animation.active()) { animation.update(); }
	state_function = state_function();
}

int PlayerAnimation::get_frame() const { return animation.get_frame(); }

fsm::StateFunction PlayerAnimation::update_idle() { 
	animation.set_params(idle);
	return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_run() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_jumpsquat() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_turn() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_rise() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_suspend() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_fall() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_stop() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_inspect() { return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_land() { return std::move(state_function); }

} // namespace player
