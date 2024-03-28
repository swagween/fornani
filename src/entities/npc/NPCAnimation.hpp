
#pragma once

#include <iostream>
#include <optional>
#include <unordered_map>
#include "../../utils/StateFunction.hpp"
#include "../animation/Animation.hpp"
#define NPC_BIND(f) std::bind(&NPCAnimation::f, this)

namespace entity {

enum class NPCAnimState { idle, turn, walk, inspect };

class NPCAnimation {

  public:
	NPCAnimation() {
		state_function = state_function();
		animation.set_params(NPC_idle);
		animation.start();
		state.set(NPCAnimState::idle);
	}

	anim::Animation animation{};
	util::BitFlags<NPCAnimState> state{};

	void update();
	int get_frame() const;

	float run_threshold{0.02f};

	fsm::StateFunction state_function = std::bind(&NPCAnimation::update_idle, this);

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_inspect();

  	anim::Parameters NPC_idle{};
	anim::Parameters NPC_walk{};
	anim::Parameters NPC_turn{};
	anim::Parameters NPC_inspect{};
};

} // namespace entity
