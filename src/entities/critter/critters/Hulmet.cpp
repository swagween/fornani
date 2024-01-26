//
//  hulmet.cpp
//  critter
//
//

#include "Hulmet.hpp"

namespace critter {
	
	void Hulmet::unique_update() {

		state_function = state_function();
	}

	void Hulmet::load_data() {
		colliders.push_back(shape::Collider());
	}

	fsm::StateFunction Hulmet::update_idle() {
		if (behavior.params.complete) { behavior = behavior::Behavior(behavior::hulmet_idle); }
		if (flags.turning) {
			return BIND(update_turn);
		}
		return std::move(state_function);
	}

	fsm::StateFunction Hulmet::update_turn() {
		if (behavior.params.started) { behavior = behavior::Behavior(behavior::hulmet_turn); behavior.params.started = false; }
		if (behavior.params.complete) { flags.turning = false; flags.flip = true; return BIND(update_idle); }
		return std::move(state_function);
	}

} // end critter

/* hulmet_cpp */
