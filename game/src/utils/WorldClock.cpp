
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/WorldClock.hpp>

namespace fornani {

WorldClock::WorldClock() : rate{196}, transition{4096} {}

auto WorldClock::calculate_tod_from_hour() const -> TimeOfDay {
	if (increments.hours.get() == dusk_time) { return TimeOfDay::dusk; }
	if (increments.hours.get() == dawn_time) { return TimeOfDay::dawn; }
	if (increments.hours.get() >= dawn_time + 1 && increments.hours.get() < dusk_time) { return TimeOfDay::day; }
	return TimeOfDay::night;
}

auto WorldClock::calculate_i_tod_from_hour() const -> int { return static_cast<int>(calculate_tod_from_hour()); }

void WorldClock::update(automa::ServiceProvider& svc) {
	transition.update();
	if (svc.ticker.every_x_ticks(rate)) {
		increments.minutes.modulate(1);
		if (increments.minutes.cycled()) {
			bool change = is_twilight();
			increments.hours.modulate(1);
			if (is_twilight()) {
				transition.start();
				current_time_of_day.modulate(1);
			}
			if (is_daytime() && change) {
				transition.start();
				current_time_of_day.modulate(1);
			}
			if (is_nighttime() && change) {
				transition.start();
				current_time_of_day.modulate(1);
			}
		}
	}
}

void WorldClock::set_time(int hour, int minute) {
	increments.hours.set(hour);
	increments.minutes.set(minute);
	current_time_of_day.set(calculate_i_tod_from_hour());
	auto change = is_twilight();
	auto offset = transition.get_native_time() - minute * rate;
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

auto WorldClock::get_previous_time_of_day() const -> TimeOfDay {
	auto prev = current_time_of_day;
	prev.modulate(-1);
	return prev.as<TimeOfDay>();
}

std::string WorldClock::tod_as_string(TimeOfDay const tod) {
	switch (tod) {
	case TimeOfDay::dawn: return "Dawn"; break;
	case TimeOfDay::day: return "Day"; break;
	case TimeOfDay::dusk: return "Dusk"; break;
	case TimeOfDay::night: return "Night"; break;
	}
	return "<invalid>";
}

std::string WorldClock::get_string() { return get_hours_string() + ":" + get_minutes_string() + (is_military() ? "" : increments.hours.get() >= 12 ? "pm" : "am"); }

} // namespace fornani
