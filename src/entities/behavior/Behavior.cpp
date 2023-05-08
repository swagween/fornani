//
//  Behavior.cpp
//  behavior
//
//

#include "Behavior.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace behavior {
    
    void Behavior::refresh() {
        if (params.current_frame == 0) { params.complete = false; }
        if (params.current_frame >= params.duration && (!params.no_loop || params.transitional)) {
            params.current_frame = params.no_loop ? (params.duration - 1) : 0;
            params.complete = true;
        } else if (params.current_frame >= params.duration && params.no_loop && !params.transitional) {
            params.current_frame = params.duration - 1;
            params.done = true;
        }
    }
    
    void Behavior::update() {

        params.frame_trigger = false;

        dt = svc::clockLocator.get().tick_rate;
        dt *= svc::clockLocator.get().tick_multiplier * animation_multiplier;

        auto new_time = Clock::now();
        Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

        if (frame_time.count() > svc::clockLocator.get().frame_limit) {
            frame_time = Time{ svc::clockLocator.get().frame_limit };
        }
        current_time = new_time;
        accumulator += frame_time;

        int integrations = 0;
        if (accumulator >= dt) {

            params.anim_frame--;
            if (params.anim_frame < 0) {
                params.anim_frame = params.framerate - 1;
            }
            if (params.anim_frame == 0) {
                params.current_frame++;
                params.frame_trigger = true;
            }
            refresh();

            accumulator = Time::zero();
            ++integrations;
        }


        
    }
    
    int Behavior::get_frame() { return params.lookup_value + params.current_frame; }

    bool Behavior::restricted() {
		return params.restrictive && !params.complete;
    }
    


} // end behavior

/* Behavior_hpp */
