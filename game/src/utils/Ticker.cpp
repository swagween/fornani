
#include <fornani/utils/Ticker.hpp>

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
	slowdown.set_and_start(time);
	slowdown_target = target;
	slowdown_rate = rate;
}

void Ticker::freeze_frame(int time, float rate) {
	freezeframe.start(time);
	dt_scalar = 0.f;
	slowdown_rate = rate;
}

void Ticker::set_time(Sec time) {
	in_game_seconds_passed = time;
	second_ticker.elapsed = time;
	twenty_minute_ticker.elapsed = time;
}

void Ticker::scale_dt() {
	flags.set(TickerFlags::forced_slowdown);
	dt_scalar = global_scalar;
}

void Ticker::reset_dt() {
	flags.reset(TickerFlags::forced_slowdown);
	dt_scalar = 1.f;
	global_scalar = 1.f;
}

void Ticker::manage_slowdowns() {
	if (freezeframe.running()) {
		dt_scalar = 0.01f;
	} else {
		dt_scalar = ccm::ext::clamp(dt_scalar + slowdown_rate, 0.f, global_scalar);
	}
	if (slowdown.running()) { dt_scalar = ccm::ext::clamp(1.f - slowdown_target * util::slowdown(slowdown.get_normalized()), 0.f, global_scalar); }

	freezeframe.update();
	slowdown.update();
}

} // namespace fornani::util
