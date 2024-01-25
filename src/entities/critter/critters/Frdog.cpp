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

        //set colliders depending on animation state
        for(auto& hurtbox : hurtboxes) {
            //hurtbox.bounding_box.set_position(get_hurtbox_pos(i));
        }

        //hurtboxes.at(0).bounding_box.set_position(sf::Vector2<float>)

        state_function = state_function();
	}

    void Frdog::load_data() {
        constexpr auto text = R"({ "xdim": 72, "ydim": 48 })";
        auto const stats = dj::Json::parse(text);
        assert(!stats.is_null());
        sprite_dimensions.x = stats["xdim"].as<int>();
        sprite_dimensions.y = stats["ydim"].as<int>();
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
        if (flags.just_hurt) { svc::assetLocator.get().enem_hit.play(); }
        flags.just_hurt = false;
        if (behavior.start()) {
            behavior = behavior::Behavior(behavior::frdog_hurt);
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
