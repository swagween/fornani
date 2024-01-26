//
//  Animation.cpp
//  anim
//
//

#include "Animation.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace anim {
    
    void Animation::refresh() {
        if (params.current_frame == 0) { params.done = false; }
        if (params.current_frame >= params.duration && (!params.no_loop)) {
            params.current_frame = params.no_loop ? (params.duration - 1) : 0;
        } else if (params.current_frame >= params.duration && params.no_loop) {
            params.current_frame = params.duration - 1;
            params.done = true;
        }
    }

    void Animation::start() {
        params.started = true;;
    }
    
    void Animation::update() {

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

    void Animation::end(bool cutoff) {
        if (cutoff) {
            params.started = false;
            params.current_frame = 0;
        } else {
            params.started = false;
        }
    }
    
    int Animation::get_frame() { return params.current_frame; }
    


} // end anim

/* Animation_cpp */
