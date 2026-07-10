
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/utils/WorldClock.hpp>

namespace fornani {

constexpr float world_clock_transition_time_v{21.f};

WorldClock::WorldClock() : rate{196} {}

auto WorldClock::calculate_tod_from_hour() const -> TimeOfDay {
	if (increments.hours.get() == dusk_time) { return TimeOfDay::dusk; }
	if (increments.hours.get() == dawn_time) { return TimeOfDay::dawn; }
	if (increments.hours.get() >= dawn_time + 1 && increments.hours.get() < dusk_time) { return TimeOfDay::day; }
	return TimeOfDay::night;
}

auto WorldClock::calculate_i_tod_from_hour() const -> int { return static_cast<int>(calculate_tod_from_hour()); }

void WorldClock::update(automa::ServiceProvider& svc) {
	rate.update();
	if (rate.is_complete()) {
		rate.start();
		increments.minutes.modulate(1);
		if (increments.minutes.cycled()) {
			bool change = is_twilight();
			increments.hours.modulate(1);
			rng.hourly = random::random_range_float(0.f, 1.f);
			if (is_twilight()) { current_time_of_day.modulate(1); }
			if (is_daytime() && change) {
				current_time_of_day.modulate(1);
				increments.days.modulate(1);
				rng.daily = random::random_range_float(0.f, 1.f);
				if (increments.days.cycled()) { rng.weekly = random::random_range_float(0.f, 1.f); }
			}
			if (is_nighttime() && change) { current_time_of_day.modulate(1); }
		}
	}
}

void WorldClock::serialize(dj::Json& out) {
	out["days"] = get_days();
	out["hours"] = get_hours();
	out["minutes"] = get_minutes();
	out["rng"]["hourly"] = rng.hourly;
	out["rng"]["daily"] = rng.daily;
	out["rng"]["weekly"] = rng.weekly;
}

void WorldClock::unserialize(dj::Json const& in) {
	increments.days.set(in["hours"].as<int>());
	set_time(in["hours"].as<int>(), in["minutes"].as<int>());
	rng.hourly = in["rng"]["hourly"].as<float>();
	rng.daily = in["rng"]["daily"].as<float>();
	rng.weekly = in["rng"]["weekly"].as<float>();
}

void WorldClock::set_time(int hour, int minute) {
	increments.hours.set(hour);
	increments.minutes.set(minute);
	current_time_of_day.set(calculate_i_tod_from_hour());
}

void WorldClock::set_speed(int to_rate, int to_transition) { rate.set_native_time(to_rate); }

void WorldClock::toggle_military_time() { m_mode = m_mode == ClockMode::standard ? ClockMode::military : ClockMode::standard; }

void WorldClock::set_military(bool const to_military) { m_mode = to_military ? ClockMode::military : ClockMode::standard; }

void WorldClock::set_rng(WorldClockInterval interval, float to) {
	switch (interval) {
	case WorldClockInterval::week: rng.weekly = to; [[fallthrough]];
	case WorldClockInterval::day: rng.daily = to; [[fallthrough]];
	case WorldClockInterval::hour: rng.hourly = to; break;
	}
}

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

auto WorldClock::get_transition() const -> float {
	auto change = increments.hours.get() == dawn_time || increments.hours.get() == dawn_time + 1 || increments.hours.get() == dusk_time || increments.hours.get() == dusk_time + 1;
	if (!change) { return 0.f; }
	auto t = increments.minutes.as<float>() + rate.get_inverse_normalized();
	t = std::clamp(t, 0.f, world_clock_transition_time_v);
	auto tt = t / world_clock_transition_time_v;
	return std::clamp(1.f - tt, 0.f, 1.f);
}

auto WorldClock::get_rng(WorldClockInterval interval) const -> float {
	switch (interval) {
	case WorldClockInterval::week: return rng.weekly;
	case WorldClockInterval::day: return rng.daily;
	case WorldClockInterval::hour: return rng.hourly;
	}
	return 0.f;
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
