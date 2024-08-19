#include "Jump.hpp"

namespace player {

void Jump::update() {
	cooldown.update();
	if (!cooldown.is_complete()) {
		states.set(JumpState::jump_began);
		coyote_time.cancel();
	} else {
		states.reset(JumpState::jump_began);
	}
	request.update();
	coyote_time.update();
}

void Jump::reset_triggers() { triggers = {}; }

void Jump::reset_all() {
	triggers = {};
	states = {};
}

void Jump::request_jump() { request.start(request_time); }

void Jump::prevent() { request.cancel(); }

void Jump::doublejump() { jump_counter.update(); }

void Jump::start() {
	cooldown.start(cooldown_time);
	states.set(player::JumpState::jumping);
	triggers.reset(player::JumpTrigger::just_jumped);
}

void Jump::reset() {
	states.reset(player::JumpState::jumping);
	triggers.reset(player::JumpTrigger::is_released);
}

void Jump::start_jumpsquat() {
	triggers.set(player::JumpTrigger::jumpsquat);
	states.set(player::JumpState::jumpsquatting);
}

void Jump::stop_jumpsquatting() { states.reset(player::JumpState::jumpsquatting); }

void Jump::reset_jumpsquat_trigger() { triggers.reset(player::JumpTrigger::jumpsquat); }

void Jump::reset_just_jumped() { triggers.reset(player::JumpTrigger::just_jumped); }

void Jump::reset_jumping() { states.reset(player::JumpState::jumping); }

bool Jump::requested() const { return !request.is_complete(); }

bool Jump::launched() const { return !cooldown.is_complete(); }

bool Jump::released() const { return triggers.test(player::JumpTrigger::is_released) && states.test(player::JumpState::jumping); }

bool Jump::began() const { return states.test(player::JumpState::jump_began); }

bool Jump::can_jump() const { return states.test(player::JumpState::can_jump); }

bool Jump::jumping() const { return states.test(player::JumpState::jumping); }

bool Jump::just_jumped() const { return triggers.test(player::JumpTrigger::just_jumped); }

bool Jump::held() const { return states.test(player::JumpState::jump_held); }

bool Jump::jumpsquatting() const { return states.test(player::JumpState::jumpsquatting); }

bool Jump::jumpsquat_trigger() const { return triggers.test(player::JumpTrigger::jumpsquat); }

int Jump::get_request() const { return request.get_cooldown(); }

int Jump::get_cooldown() const { return cooldown.get_cooldown(); }

} // namespace player
