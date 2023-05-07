//
//  frdog.cpp
//  critter
//
//

#include "Frdog.hpp"

namespace critter {

	void Frdog::unique_update() {
        /*if (flags.asleep) {
            behavior.sleep();
            current_target = collider.physics.position;
        }
        else if (flags.awakened) {
            behavior.awaken();
            current_target = collider.physics.position;
        }
        else {
            behavior.idle();
        }

        if (collider.flags.test(shape::State::is_any_jump_collision)) {
            collider.physics.acceleration.y = 0.0f;
            collider.flags.reset(shape::State::is_any_jump_collision);
        }

        if (collider.physics.acceleration.x < 0.0f && collider.physics.velocity.x > 0.0f) { update_turn(); behavior.facing_lr = behavior::DIR_LR::LEFT; }
        if (collider.physics.acceleration.x > 0.0f && collider.physics.velocity.x < 0.0f) { update_turn(); behavior.facing_lr = behavior::DIR_LR::RIGHT; }


        if (abs(collider.physics.velocity.x) > 0.02f) {
            if (behavior.ready()) { behavior.current_state = behavior::Behavior(behavior::frdog_run); }
        }*/

        state_function = state_function();

	}

	fsm::StateFunction Frdog::update_start() {
        printf("starting...\n");
        return BIND(update_idle);
	}

	fsm::StateFunction Frdog::update_idle() {
        if (behavior.params.complete) { behavior = behavior::Behavior(behavior::frdog_idle); }
		if (abs(collider.physics.velocity.x) > 0.02f) {
			return BIND(update_run);
		}
        if (flags.awakened) {
            return BIND(update_sit);
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
        behavior = behavior::Behavior(behavior::frdog_awakened);
        if(!flags.awakened) {
            return flags.asleep ? BIND(update_sleep) : BIND(update_idle);
        }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_turn() {
        //if(behavior.ready()) { behavior.current_state = behavior::Behavior(behavior::frdog_turn); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_charge() {
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_run() {
        //if(behavior.ready()) { behavior.current_state = behavior::Behavior(behavior::frdog_run); }
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
