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
}

void WorldClock::set_speed(int to_rate, int to_transition) {
	rate = to_rate;
	if (to_transition != 4096) { transition = util::Cooldown(to_transition); }
}

std::string WorldClock::get_string(bool military) {
	std::string ret{};
	std::string leading_zero = increments.minutes.get() < 10 ? "0" : "";
	std::string twelve_hour = increments.hours.get() % 12 == 0 ? "12" : std::to_string(increments.hours.get() % 12);
	std::string qualifier = increments.hours.get() >= 12 ? "pm" : "am";
	if (military) {
		ret = std::to_string(increments.hours.get()) + ":" + leading_zero + std::to_string(increments.minutes.get());
	} else {
		ret = twelve_hour + ":" + leading_zero + std::to_string(increments.minutes.get()) + qualifier;
	}
	return ret;
}

} // namespace fornani