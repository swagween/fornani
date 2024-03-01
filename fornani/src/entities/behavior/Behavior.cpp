
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

bool Behavior::start() { return (params.complete || params.started); }

void Behavior::update() {

	params.frame_trigger = false;
	params.just_started = false;

	params.anim_frame--;
	if (params.anim_frame < 0) { params.anim_frame = params.framerate - 1; }
	if (params.anim_frame == 0) {
		params.current_frame++;
		params.frame_trigger = true;
	}
	refresh();

}

int Behavior::get_frame() { return params.lookup_value + params.current_frame; }

bool Behavior::restricted() { return params.restrictive && !params.complete; }

} // namespace behavior
