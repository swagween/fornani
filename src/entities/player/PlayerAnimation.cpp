#include "PlayerAnimation.hpp"

namespace player {

void PlayerAnimation::update() {
	animation.update();
	state_function = state_function();
}

void PlayerAnimation::start() { animation.start(); }

int PlayerAnimation::get_frame() const { return animation.get_frame(); }

bool PlayerAnimation::not_jumping() { return !(state.test(AnimState::jumpsquat) || state.test(AnimState::rise)); }

fsm::StateFunction PlayerAnimation::update_idle() {
	animation.label = "idle";
	if (animation.counter > timers.sit) { state.set(AnimState::sit); }
	if (change_state(AnimState::sit, sit)) { return PA_BIND(update_sit); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	state = {};
	state.set(AnimState::idle);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_sprint() {
	animation.label = "sprint";
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	state = {};
	state.set(AnimState::sprint);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_shield() {
	animation.label = "shield";
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	state = {};
	state.set(AnimState::shield);
	return PA_BIND(update_shield);
}

fsm::StateFunction PlayerAnimation::update_run() {
	animation.label = "run";
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	state = {};
	state.set(AnimState::run);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_jumpsquat() {
	animation.label = "jumpsquat";
	if (animation.complete()) {
		if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
		state = {};
		state.set(AnimState::rise);
		animation.set_params(rise);
		return PA_BIND(update_rise);
	}
	state = {};
	state.set(AnimState::jumpsquat);
	return PA_BIND(update_jumpsquat);
}

fsm::StateFunction PlayerAnimation::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
		if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::turn);
	return PA_BIND(update_turn);
}

fsm::StateFunction PlayerAnimation::update_sharp_turn() {
	animation.label = "sharp_turn";
	if (animation.complete()) {
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::sharp_turn);
	return PA_BIND(update_sharp_turn);
}

fsm::StateFunction PlayerAnimation::update_rise() {
	animation.label = "rise";
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	state = {};
	state.set(AnimState::rise);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_suspend() {
	animation.label = "suspend";
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	state = {};
	state.set(AnimState::suspend);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_fall() {
	animation.label = "fall";
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	state = {};
	state.set(AnimState::fall);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_stop() {
	animation.label = "stop";

	if (animation.complete()) {
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	state = {};
	state.set(AnimState::stop);
	return PA_BIND(update_stop);
}

fsm::StateFunction PlayerAnimation::update_inspect() {
	animation.label = "inspect";
	if (animation.complete()) {
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	}
	if (change_state(AnimState::jumpsquat, jumpsquat)) {
		animation.end();
		return PA_BIND(update_jumpsquat);
	}
	if (change_state(AnimState::dash, dash)) {
		animation.end();
		return PA_BIND(update_dash);
	}
	state = {};
	state.set(AnimState::inspect);
	return PA_BIND(update_inspect);
}

fsm::StateFunction PlayerAnimation::update_sit() {
	animation.label = "sit";
	if (animation.complete()) {
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	}
	if (change_state(AnimState::jumpsquat, jumpsquat)) {
		animation.end();
		return PA_BIND(update_jumpsquat);
	}
	if (change_state(AnimState::dash, dash)) {
		animation.end();
		return PA_BIND(update_dash);
	}
	state = {};
	state.set(AnimState::sit);
	return PA_BIND(update_sit);
}

fsm::StateFunction PlayerAnimation::update_land() {
	animation.label = "land";
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (animation.complete()) {
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::land);
	return PA_BIND(update_land);
}

fsm::StateFunction PlayerAnimation::update_hurt() {
	animation.label = "hurt";
	if (animation.complete()) {
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
		if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	state = {};
	state.set(AnimState::hurt);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_dash() {
	animation.label = "dash";
	if (animation.complete()) {
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::jumpsquat, jumpsquat)) { return PA_BIND(update_jumpsquat); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::dash);
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_wallslide() {
	animation.label = "wallslide";
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	state = {};
	state.set(AnimState::wallslide);
	return PA_BIND(update_wallslide);
}

bool PlayerAnimation::change_state(AnimState next, anim::Parameters params) {
	if (state.test(next)) {
		state = {};
		animation.set_params(params);
		return true;
	}
	return false;
 }

} // namespace player
