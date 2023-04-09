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
        if(params.current_frame >= params.duration) {
            params.current_frame = 0;
            params.complete = true;
        }
    }
    
    void Behavior::update() {

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
            }
            refresh();

            accumulator = Time::zero();
            ++integrations;
        }


        
    }
    
    int Behavior::get_frame() { return params.lookup_value + params.current_frame; }
    


} // end behavior

/* Behavior_hpp */