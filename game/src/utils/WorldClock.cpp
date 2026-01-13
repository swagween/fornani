
#include "fornani/utils/WorldClock.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani {

WorldClock::WorldClock() : rate{196}, transition{4096} {}

void WorldClock::update(automa::ServiceProvider& svc) {
	transition.update();
	if (svc.ticker.every_x_ticks(rate)) {
		increments.minutes.modulate(1);
		if (increments.minutes.cycled()) {
			bool change = is_twilight();
			previous_time_of_day = get_time_of_day();
			increments.hours.modulate(1);
			if (is_twilight()) { transition.start(); }
			if (is_daytime() && change) { transition.start(); }
			if (is_nighttime() && change) { transition.start(); }
		}
	}
}

void WorldClock::set_time(int hour, int minute) {
	increments.hours.set(hour);
	increments.minutes.set(minute);
	auto change = is_twilight();
	auto offset = transition.get_native_time() - minute * rate;
	previous_time_of_day = get_time_of_day();
	if (is_twilight()) { transition.start(offset); }
	if (is_daytime() && change) { transition.start(offset); }
	if (is_nighttime() && change) { transition.start(offset); }
}

void WorldClock::set_speed(int to_rate, int to_transition) {
	rate = to_rate;
	if (to_transition != 4096) { transition = util::Cooldown(to_transition); }
}

void WorldClock::toggle_military_time() { m_mode = m_mode == ClockMode::standard ? ClockMode::military : ClockMode::standard; }

std::string WorldClock::get_hours_string() {
	std::string ret{};
	std::string twelve_hour = increments.hours.get() % 12 == 0 ? "12" : std::to_string(increments.hours.get() % 12);
	ret = is_military() ? std::to_string(increments.hours.get()) : twelve_hour;
	return ret;
}

std::string WorldClock::get_minutes_string() { return (increments.minutes.get() < 10 ? "0" : "") + std::to_string(increments.minutes.get()); }

std::string WorldClock::get_string() { return get_hours_string() + ":" + get_minutes_string() + (is_military() ? "" : increments.hours.get() >= 12 ? "pm" : "am"); }

} // namespace fornani
