
#include "Animation.hpp"
#include <iostream>

namespace anim {

void Animation::refresh() {
	frame.start();
	loop.start();
	flags.reset(State::param_switch);
	flags.reset(State::complete);
	start();
}

void Animation::start() {
	flags.set(State::just_started);
	frame.start();
	global_counter.start();
	frame_timer.start(params.framerate);
}

void Animation::update() {
	if (frame_timer.is_complete()) {
		frame.update();
		if (frame.get_count() == params.duration) {
			if (params.repeat_last_frame) {
				frame.set(params.duration - 1);
				end();
				return;
			}
			if (loop.get_count() == params.num_loops) {
				loop.start();
				end();
				return;
			} else {
				frame.start();
				loop.update();
			}
		}
		if (!params.repeat_last_frame) { frame_timer.start(params.framerate); }
	}
	global_counter.update();
	frame_timer.update();
}

void Animation::end() { flags.set(State::complete); }

void Animation::set_params(Parameters const new_params) {
	next_params = new_params;
	switch_params();
}

void Animation::switch_params() {
	params = next_params;
	refresh();
}

int Animation::get_frame() const { return params.lookup + frame.get_count(); }

bool Animation::active() const { return flags.test(State::active); }

bool Animation::complete() const { return flags.test(State::complete); }

bool Animation::keyframe_over() const { return params.framerate != 0 ? global_counter.get_count() % params.framerate == 0 : true; } //long-winded to avoid division by 0

} // namespace anim
