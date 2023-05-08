//
//  frdog.cpp
//  critter
//
//

#include "Frdog.hpp"

namespace critter {

	void Frdog::unique_update() {

        seek_current_target();

        state_function = state_function();

	}

	fsm::StateFunction Frdog::update_idle() {
        if (behavior.params.complete) { behavior = behavior::Behavior(behavior::frdog_idle); }
		if (abs(collider.physics.velocity.x) > 0.02f) {
			return BIND(update_run);
		}
        if (flags.awakened) {
            return BIND(update_sit);
        }
        if (flags.turning) {
            return BIND(update_turn);
        }
        return std::move(state_function);
	}

    fsm::StateFunction Frdog::update_sleep() {

        if (behavior.params.complete) { behavior = behavior::Behavior(behavior::frdog_asleep); }
        if(flags.awakened) {
            return BIND(update_sit);
        }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_sit() {
        if (behavior.params.complete) { behavior = behavior::Behavior(behavior::frdog_awakened); }
        if(!flags.awakened) {
            return flags.asleep ? BIND(update_sleep) : BIND(update_idle);
        }
        return std::move(state_function);
    }
     
    fsm::StateFunction Frdog::update_turn() {
        if (behavior.params.started) { behavior = behavior::Behavior(behavior::frdog_turn); behavior.params.started = false; }
        if (behavior.params.complete) { flags.turning = false; flags.flip = true; return BIND(update_idle); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_charge() {
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_run() {
        if(behavior.params.complete) { behavior = behavior::Behavior(behavior::frdog_run); }
        if (flags.turning) { return BIND(update_turn); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_hurt() {
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_bark() {
        return std::move(state_function);
    }

} // end components

/* frdog_cpp */
