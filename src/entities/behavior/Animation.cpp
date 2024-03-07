
#include "Animation.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace anim {

void Animation::refresh() {
	counter = 0;
	current_frame = 0;
	loop_counter = 0;
}

void Animation::start() {
	flags.set(State::active);
	flags.reset(State::complete);
}

void Animation::update() {
	++counter;

	if (keyframe_over()) {
		++current_frame;
	}

	if (current_frame == params.duration && params.repeat_last_frame) {
		current_frame = params.duration - 1;
		end();
		return;
	}
	if (current_frame == params.duration) {
		if (loop_counter == params.num_loops) { end(); }
		current_frame = 0;
		if (params.num_loops != -1) {
			++loop_counter;
			// don't increment for infinite loops
		}
	}
}

void Animation::end() {
	flags.reset(State::active);
	flags.set(State::complete);
}

void Animation::set_params(Parameters& const new_params) {
	params = new_params;
	refresh();
	start();
}

int Animation::get_frame() const { return params.lookup + current_frame; }

bool Animation::active() const { return flags.test(State::active); }

bool Animation::complete() const { return flags.test(State::complete); }

bool Animation::keyframe_over() const { return params.framerate != 0 ? counter % params.framerate == 0 : true; } //long-winded to avoid division by 0

} // namespace anim
