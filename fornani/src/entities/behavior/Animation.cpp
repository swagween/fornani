
#include "Animation.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace anim {

void Animation::refresh() {
	counter = 0;
	current_frame = 0;
}

void Animation::start() {
	flags.set(State::active);
	flags.reset(State::complete);
}

void Animation::update() {
	++counter;
	if (keyframe_over()) {
		//std::cout << "frame: " << current_frame << "\n";
		++current_frame;
	}
	if (current_frame == params.duration) {
		current_frame = 0;
		if (params.one_off) { end(); }
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

bool Animation::keyframe_over() const { return counter % params.framerate == 0; }

} // namespace anim
