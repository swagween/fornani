
#pragma once
#include "Ticker.hpp"

namespace util {

void Ticker::start_frame() {
	++num_frames;
	total_seconds_passed += dt;
	total_milliseconds_passed += dt * 1000.f;
}

void Ticker::end_frame() {
	ticks_per_frame = total_integrations / (num_frames * calls_per_frame);
	calls_per_frame = 0;
	calculate_fps();
}

void Ticker::calculate_fps() {
	if (num_frames <= sample_size) {
		frame_list.push_back(dt);
	} else {
		frame_list.pop_front();
		frame_list.push_back(dt);
	}
	for (auto& frame : frame_list) { seconds_passed += frame; }

	fps = num_frames <= sample_size ? num_frames / seconds_passed.count() : sample_size / seconds_passed.count();
	seconds_passed = Tim::zero();
}

void Ticker::slow_down(int time) { slowdown.start(time); }

void Ticker::freeze_frame(int time) { freezeframe.start(time); }

} // namespace util