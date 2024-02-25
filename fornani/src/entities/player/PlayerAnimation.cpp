#include "PlayerAnimation.hpp"

namespace player {

	

void PlayerAnimation::update() {
	animation.update();
	state_function = state_function();
}

void PlayerAnimation::start() { animation.start(); }

int PlayerAnimation::get_frame() const { return animation.get_frame(); }

fsm::StateFunction PlayerAnimation::update_idle() { 
	animation.label = "idle";
	if (state.test(AnimState::turn)) {
		animation.set_params(turn);
		animation.refresh();
		return BIND(update_turn);
	}
	if (state.test(AnimState::jumpsquat)) {
		animation.set_params(jumpsquat);
		animation.refresh();
		return BIND(update_jumpsquat);
	}
	if (state.test(AnimState::run)) {
		animation.set_params(run);
		animation.refresh();
		return BIND(update_run);
	}
	return std::move(state_function); }

fsm::StateFunction PlayerAnimation::update_run() {
	animation.label = "run";
	if (state.test(AnimState::stop)) {
		animation.set_params(stop);
		animation.refresh();
		return BIND(update_stop);
	}
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_jumpsquat() {
	animation.label = "jumpsquat";
	if (animation.complete()) {
		animation.set_params(rise);
		state.reset(AnimState::jumpsquat);
		state.set(AnimState::rise);
		animation.start();
		return BIND(update_rise);
	}
	return BIND(update_jumpsquat);
}

fsm::StateFunction PlayerAnimation::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		animation.set_params(idle);
		state.reset(AnimState::turn);
		state.set(AnimState::idle);
		animation.start();
		return BIND(update_idle);
	}
	return BIND(update_turn);
}

fsm::StateFunction PlayerAnimation::update_rise() {
	animation.label = "rise";
	if (state.test(AnimState::suspend)) {
		animation.set_params(suspend);
		animation.refresh();
		return BIND(update_suspend);
	}
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_suspend() {
	animation.label = "suspend";
	if (state.test(AnimState::fall)) {
		animation.set_params(fall);
		animation.refresh();
		return BIND(update_fall);
	}
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_fall() {
	animation.label = "fall";
	if (state.test(AnimState::land)) {
		animation.set_params(land);
		animation.refresh();
		return BIND(update_land);
	}
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_stop() {
	animation.label = "stop";
	if (animation.complete()) {
		animation.set_params(idle);
		state.reset(AnimState::stop);
		state.set(AnimState::idle);
		animation.start();
		return BIND(update_idle);
	}
	return BIND(update_stop);
}

fsm::StateFunction PlayerAnimation::update_inspect() {
	animation.label = "inspect";
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_land() {
	animation.label = "land";
	if (animation.complete()) {
		animation.set_params(idle);
		state.reset(AnimState::land);
		state.set(AnimState::idle);
		animation.start();
		return BIND(update_idle);
	}
	return BIND(update_land);
}

} // namespace player
