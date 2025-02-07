
#include "fornani/entities/animation/Animation.hpp"
#include <iostream>

namespace fornani::anim {

void Animation::refresh() {
	frame.start();
	loop.start();
	flags.reset(State::param_switch);
	flags.reset(State::keyframe);
}

void Animation::start() {
	frame.start();
	global_counter.start();
	loop.start();
}

void Animation::update() {
	frame_timer.update();
	global_counter.update();
	if (params.interruptible && flags.test(State::param_switch)) {
		switch_params();
		return;
	}
	if (frame_timer.is_complete()) { // next frame of animation

		// reset the timers
		flags.set(State::keyframe);
		frame_timer.start(params.framerate);
		frame.update(); // increment frame

		// frame is over, so we can switch params if requested
		if (flags.test(State::param_switch)) {
			switch_params();
			return;
		}

		if (frame.get_count() == params.duration) { // we hit the end of the animation

			// for one-off animations, set oneoff flag and leave
			if (params.num_loops == 0) {
				frame.set(params.duration - 1);
				flags.set(State::oneoff_complete);
			} else {
				// for animations that loop indefinitely, just reset
				frame.start();
			}

			// for animations with multiple loops, increment the loop counter and start over
			if (params.num_loops > 0) {
				if(params.num_loops == loop.get_count()) { // final loop ended
					frame.cancel();
				}
				loop.update();
			}

			// for animations where we repeat the last frame, fix it to the last frame
			// we do this at the end to overwrite the state of `frame`
			if (params.repeat_last_frame) {
				frame.set(params.duration - 1);
			}
		}
	}
}

void Animation::set_params(Parameters const new_params, bool hard) {
	next_params = new_params;
	if (hard) {
		switch_params();
		return;
	}
	flags.set(State::param_switch);
}

void Animation::switch_params() {
	params = next_params;
	global_counter.start();
	frame_timer.start(params.framerate);
	refresh();
}

int Animation::get_frame() const { return frame.canceled() ? params.lookup : params.lookup + frame.get_count(); }

} // namespace anim
