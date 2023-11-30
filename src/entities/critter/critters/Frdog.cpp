//
//  frdog.cpp
//  critter
//
//

#include "Frdog.hpp"

namespace critter {

	void Frdog::unique_update() {

        //seek_current_target();
        random_idle_action();
        while (!idle_action_queue.empty()) {
            flags.barking = true;
            idle_action_queue.pop();
        }

        state_function = state_function();
	}

	fsm::StateFunction Frdog::update_idle() {
        if (behavior.start()) { behavior = behavior::Behavior(behavior::frdog_idle); behavior.params.started = false; }
		if (abs(collider.physics.velocity.x) > 0.0002f) { behavior.params.started = true; return BIND(update_run); }
        if (flags.awakened)     { behavior.params.started = true; return BIND(update_sit); }
        if (flags.turning)      { behavior.params.started = true; return BIND(update_turn); }
        if (flags.barking)      { behavior.params.started = true; return BIND(update_bark); }
        if (flags.hurt)         { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
	}

    fsm::StateFunction Frdog::update_sleep() {

        if (behavior.start()) { behavior = behavior::Behavior(behavior::frdog_asleep); behavior.params.started = false;
        }
        if(flags.awakened) { behavior.params.started = true; return BIND(update_sit); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_sit() {
        if (behavior.start()) { behavior = behavior::Behavior(behavior::frdog_awakened); behavior.params.started = false; }
        if(!flags.awakened) { behavior.params.started = true; return flags.asleep ? BIND(update_sleep) : BIND(update_idle); }
        if (flags.hurt) {behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }
     
    fsm::StateFunction Frdog::update_turn() {
        if (behavior.params.started) { behavior = behavior::Behavior(behavior::frdog_turn); behavior.params.started = false; }
        if (behavior.params.complete) { flags.turning = false; flags.flip = true; return BIND(update_idle); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_charge() {
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_run() {
        if(behavior.start()) { behavior = behavior::Behavior(behavior::frdog_run); behavior.params.started = false; }
        if (flags.turning) { behavior.params.started = true; return BIND(update_turn); }
        if (abs(collider.physics.velocity.x) < 0.04f) { behavior.params.just_started = true; return BIND(update_idle); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_hurt() {
        if (behavior.start()) {
            behavior = behavior::Behavior(behavior::frdog_hurt); 
            if (anim_loop_count == 0) { svc::assetLocator.get().enem_hit.play(); }
            behavior.params.started = false;
            ++anim_loop_count;
        }

        if (anim_loop_count > 2) {
            behavior.params.started = true;
            anim_loop_count = 0;
            offset = { 0.0f, 8.0f };
            flags.hurt = false;
            return flags.shot ? BIND(update_hurt) : BIND(update_idle);
        }

        return std::move(state_function);
    }

    fsm::StateFunction Frdog::update_bark() {
        if (behavior.params.started) { behavior = behavior::Behavior(behavior::frdog_bark); behavior.params.started = false; }
        if (behavior.params.complete) { behavior.params.started = true; flags.barking = false; return BIND(update_idle); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

} // end components

/* frdog_cpp */
