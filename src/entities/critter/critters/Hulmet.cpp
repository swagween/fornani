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

	fsm::StateFunction Hulmet::update_start() {
		//behavior.current_state = behavior::Behavior(behavior::hulmet_run);
		return std::move(state_function);
	}

	fsm::StateFunction Hulmet::update_idle() {
		if (behavior.params.complete) { behavior = behavior::Behavior(behavior::hulmet_idle); }
		return std::move(state_function);
	}
} // end critter

/* hulmet_cpp */
