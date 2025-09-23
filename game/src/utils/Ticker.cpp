
#include "fornani/utils/Ticker.hpp"

namespace fornani::util {

void Ticker::start_frame() {
	++num_frames;
	total_seconds_passed += dt;
	in_game_seconds_passed += dt;
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
	seconds_passed = Sec::zero();
}

void Ticker::slow_down(int time, float target, float rate) {
	slowdown.start(time);
	slowdown_target = target;
	slowdown_rate = rate;
}

void Ticker::freeze_frame(int time, float target) {
	freezeframe.start(time);
	slowdown_target = target;
	slowdown_rate = 0.1f;
}

void Ticker::set_time(Sec time) {
	in_game_seconds_passed = time;
	second_ticker.elapsed = time;
	twenty_minute_ticker.elapsed = time;
}

void Ticker::scale_dt() { flags.set(TickerFlags::forced_slowdown); }

void Ticker::reset_dt() {
	flags.reset(TickerFlags::forced_slowdown);
	dt_scalar = 1.f;
}

} // namespace fornani::util
