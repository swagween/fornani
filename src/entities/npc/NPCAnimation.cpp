#include "NPCAnimation.hpp"

namespace entity {

void NPCAnimation::update() {
	animation.update();
	state_function = state_function();
	state = {};
}

int NPCAnimation::get_frame() const { return animation.get_frame(); }

fsm::StateFunction NPCAnimation::update_idle() {
	animation.label = "idle";
	if (state.test(NPCAnimState::turn)) {
		state.reset(NPCAnimState::idle);
		animation.set_params(NPC_turn);
		return NPC_BIND(update_turn);
	}
	if (state.test(NPCAnimState::walk)) {
		state.reset(NPCAnimState::idle);
		animation.set_params(NPC_walk);
		return NPC_BIND(update_walk);
	}
	if (state.test(NPCAnimState::inspect)) {
		state.reset(NPCAnimState::idle);
		animation.set_params(NPC_inspect);
		return NPC_BIND(update_inspect);
	}
	state = {};
	state.set(NPCAnimState::idle);
	return std::move(state_function);
}

fsm::StateFunction NPCAnimation::update_walk() {
	animation.label = "walk";
	if (state.test(NPCAnimState::turn)) {
		state.reset(NPCAnimState::walk);
		animation.set_params(NPC_turn);
		return NPC_BIND(update_turn);
	}
	if (state.test(NPCAnimState::idle) && animation.keyframe_over()) {
		state.reset(NPCAnimState::walk);
		animation.set_params(NPC_idle);
		return NPC_BIND(update_idle);
	}
	if (state.test(NPCAnimState::inspect)) {
		state.reset(NPCAnimState::walk);
		animation.set_params(NPC_inspect);
		return NPC_BIND(update_inspect);
	}
	state = {};
	state.set(NPCAnimState::walk);
	return std::move(state_function);
}

fsm::StateFunction NPCAnimation::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		if (state.test(NPCAnimState::walk)) {
			state.reset(NPCAnimState::turn);
			animation.set_params(NPC_walk);
			return NPC_BIND(update_walk);
		}
		if (state.test(NPCAnimState::inspect)) {
			state.reset(NPCAnimState::turn);
			animation.set_params(NPC_inspect);
			return NPC_BIND(update_inspect);
		}
		state = {};
		state.set(NPCAnimState::idle);
		animation.set_params(NPC_idle);
		return NPC_BIND(update_idle);
	}
	state = {};
	state.set(NPCAnimState::turn);
	return NPC_BIND(update_turn);
}

fsm::StateFunction NPCAnimation::update_inspect() {
	animation.label = "inspect";
	if (animation.complete()) {
		if (state.test(NPCAnimState::walk)) {
			state.reset(NPCAnimState::inspect);
			animation.set_params(NPC_walk);
			animation.end();
			return NPC_BIND(update_walk);
		}
		if (state.test(NPCAnimState::idle)) {
			state.reset(NPCAnimState::inspect);
			animation.set_params(NPC_idle);
			animation.end();
			return NPC_BIND(update_idle);
		}
	}
	state = {};
	state.set(NPCAnimState::inspect);
	return NPC_BIND(update_inspect);
}
} // namespace entity
