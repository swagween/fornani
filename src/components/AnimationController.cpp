//
//  AnimationController.hpp
//  components
//
//

#include "AnimationController.hpp"
#include "../setup/ServiceLocator.hpp"

namespace components {

    void AnimationController::refresh() {
        if (sm.current_state->current_frame == 0) { sm.current_state->complete = false; }
        if (sm.current_state->current_frame >= sm.current_state->duration && (!sm.current_state->no_loop || sm.current_state->transitional)) {
            sm.current_state->current_frame = sm.current_state->no_loop ? (sm.current_state->duration - 1) : 0;
            sm.current_state->complete = true;
        }
        else if (sm.current_state->current_frame >= sm.current_state->duration && sm.current_state->no_loop && !sm.current_state->transitional) {
            sm.current_state->current_frame = sm.current_state->duration - 1;
            sm.current_state->done = true;
        }
    }

    void AnimationController::update() {

        sm.current_state->frame_trigger = false;

        dt = svc::clockLocator.get().tick_rate;
        dt *= svc::clockLocator.get().tick_multiplier * animation_multiplier;

        auto new_time = Clock::now();
        Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

        if (frame_time.count() > svc::clockLocator.get().frame_limit) {
            frame_time = Time{ svc::clockLocator.get().frame_limit };
        }
        current_time = new_time;
        accumulator += frame_time;

        if (accumulator >= dt) {

            sm.current_state->anim_frame--;
            if (sm.current_state->anim_frame < 0) {
                sm.current_state->anim_frame = sm.current_state->framerate - 1;
            }
            if (sm.current_state->anim_frame == 0) {
                sm.current_state->current_frame++;
                sm.current_state->frame_trigger = true;
            }
            refresh();

            accumulator = Time::zero();
        }

    }

    int AnimationController::get_frame() { return sm.current_state->lookup_value + sm.current_state->current_frame; }

} // end components

/* AnimationController_hpp */
