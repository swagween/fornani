
#pragma once

#include <fornani/core/Common.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <cstdint>
#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {

constexpr int dawn_time{6};
constexpr int dusk_time{18};

class WorldClock {
  public:
	WorldClock();
	void update(automa::ServiceProvider& svc);
	void set_time(int hour = 0, int minute = 0);
	void set_speed(int to_rate, int to_transition = 4096);
	void toggle_military_time();
	[[nodiscard]] auto is_military() const -> bool { return m_mode == ClockMode::military; }
	[[nodiscard]] auto get_normalized_time() const -> float { return static_cast<float>(get_hours() * 60 + get_minutes()) / 1440.f; }
	[[nodiscard]] auto is_daytime() const -> bool { return calculate_tod_from_hour() == TimeOfDay::day; }
	[[nodiscard]] auto is_nighttime() const -> bool { return calculate_tod_from_hour() == TimeOfDay::night; }
	[[nodiscard]] auto is_dusk() const -> bool { return calculate_tod_from_hour() == TimeOfDay::dusk; }
	[[nodiscard]] auto is_dawn() const -> bool { return calculate_tod_from_hour() == TimeOfDay::dawn; }
	[[nodiscard]] auto is_twilight() const -> bool { return is_dawn() || is_dusk(); }
	[[nodiscard]] auto is_transitioning() const -> bool { return transition.running(); }
	[[nodiscard]] auto get_time_of_day() const -> TimeOfDay { return current_time_of_day.as<TimeOfDay>(); }
	[[nodiscard]] auto get_previous_time_of_day() const -> TimeOfDay;
	[[nodiscard]] auto get_transition() const -> float { return transition.get_normalized(); }
	[[nodiscard]] auto get_hours() const -> int { return increments.hours.get(); }
	[[nodiscard]] auto get_minutes() const -> int { return increments.minutes.get(); }
	[[nodiscard]] auto get_rate() const -> int { return rate; }
	[[nodiscard]] auto as_trio() const -> int { return is_daytime() ? 0 : is_nighttime() ? 2 : 1; }
	[[nodiscard]] auto get_previous_as_trio() const -> int { return get_previous_time_of_day() == TimeOfDay::day ? 0 : get_previous_time_of_day() == TimeOfDay::night ? 2 : 1; }
	std::string tod_as_string(TimeOfDay const tod);
	std::string get_string();
	std::string get_hours_string();
	std::string get_minutes_string();

  private:
	[[nodiscard]] auto calculate_tod_from_hour() const -> TimeOfDay;
	[[nodiscard]] auto calculate_i_tod_from_hour() const -> int;
	ClockMode m_mode{};
	struct {
		util::Circuit hours{24};
		util::Circuit minutes{60};
	} increments;
	util::Circuit current_time_of_day{4};
	int rate{};
	util::Cooldown transition{};
};

} // namespace fornani
