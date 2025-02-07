#include "fornani/entities/npc/NPCAnimation.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include <iostream>

namespace fornani::npc {

NPCAnimation::NPCAnimation(automa::ServiceProvider& svc, int id) {

	m_label = svc.tables.npc_label.at(id);
	auto const& in_data = svc.data.npc[m_label];

	NPC_idle.lookup = in_data["animation"]["idle"][0].as<int>();
	NPC_idle.duration = in_data["animation"]["idle"][1].as<int>();
	NPC_idle.framerate = in_data["animation"]["idle"][2].as<int>();
	NPC_idle.num_loops = in_data["animation"]["idle"][3].as<int>();
	NPC_idle.repeat_last_frame = static_cast<bool>(in_data["animation"]["idle"][4].as_bool());

	NPC_walk.lookup = in_data["animation"]["walk"][0].as<int>();
	NPC_walk.duration = in_data["animation"]["walk"][1].as<int>();
	NPC_walk.framerate = in_data["animation"]["walk"][2].as<int>();
	NPC_walk.num_loops = in_data["animation"]["walk"][3].as<int>();
	NPC_walk.repeat_last_frame = static_cast<bool>(in_data["animation"]["walk"][4].as_bool());

	NPC_turn.lookup = in_data["animation"]["turn"][0].as<int>();
	NPC_turn.duration = in_data["animation"]["turn"][1].as<int>();
	NPC_turn.framerate = in_data["animation"]["turn"][2].as<int>();
	NPC_turn.num_loops = in_data["animation"]["turn"][3].as<int>();
	NPC_turn.repeat_last_frame = static_cast<bool>(in_data["animation"]["turn"][4].as_bool());

	NPC_inspect.lookup = in_data["animation"]["inspect"][0].as<int>();
	NPC_inspect.duration = in_data["animation"]["inspect"][1].as<int>();
	NPC_inspect.framerate = in_data["animation"]["inspect"][2].as<int>();
	NPC_inspect.num_loops = in_data["animation"]["inspect"][3].as<int>();
	NPC_inspect.repeat_last_frame = static_cast<bool>(in_data["animation"]["inspect"][4].as_bool());

	animation_flags.set(NPCAnimState::idle);
	animation.set_params(NPC_idle);
	animation.start();
	state_function = state_function();
}

void NPCAnimation::update() {
	animation.update();
	state_function = state_function();
}

fsm::StateFunction NPCAnimation::update_idle() {
	animation.label = "idle";
	if (animation_flags.test(NPCAnimState::turn)) {
		animation_flags.reset(NPCAnimState::idle);
		animation.set_params(NPC_turn);
		return NPC_BIND(update_turn);
	}
	if (animation_flags.test(NPCAnimState::walk)) {
		animation_flags.reset(NPCAnimState::idle);
		animation.set_params(NPC_walk);
		return NPC_BIND(update_walk);
	}
	animation_flags = {};
	animation_flags.set(NPCAnimState::idle);
	return std::move(state_function);
}

fsm::StateFunction NPCAnimation::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		communication_flags.set(NPCCommunication::sprite_flip);
		animation_flags = {};
		animation_flags.set(NPCAnimState::idle);
		animation.set_params(NPC_idle);
		return NPC_BIND(update_idle);
	}
	animation_flags = {};
	animation_flags.set(NPCAnimState::turn);
	return std::move(state_function);
}

fsm::StateFunction NPCAnimation::update_walk() {
	animation.label = "walk";
	if (animation_flags.test(NPCAnimState::turn)) {
		animation_flags = {};
		animation.set_params(NPC_turn);
		return NPC_BIND(update_turn);
	}
	if (animation_flags.test(NPCAnimState::idle)) {
		animation_flags = {};
		animation.set_params(NPC_idle);
		return NPC_BIND(update_idle);
	}
	animation_flags = {};
	animation_flags.set(NPCAnimState::walk);
	//std::cout << "w";
	return std::move(state_function);
}

fsm::StateFunction NPCAnimation::update_inspect() {
	animation.label = "inspect";
	animation_flags = {};
	animation_flags.set(NPCAnimState::inspect);
	return NPC_BIND(update_inspect);
}

} // namespace npc
