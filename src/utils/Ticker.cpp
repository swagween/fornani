
#pragma once
#include "Ticker.hpp"

namespace util {

float Ticker::global_tick_rate() { return ft.count() * tick_multiplier; }

void Ticker::start_frame() {
	++num_frames;
	total_seconds_passed += dt;
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

bool Ticker::every_x_frames(int const freq) { return num_frames % freq == 0; }

bool Ticker::every_x_ticks(int const freq) { return (int)total_integrations % freq == 0; }

bool Ticker::every_x_milliseconds(int num_milliseconds) { return false; }

} // namespace util