
#pragma once

#include "Circuit.hpp"
#include "Cooldown.hpp"
#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {

enum class TimeOfDay { day, twilight, night, END };

class WorldClock {
  public:
	WorldClock();
	void update(automa::ServiceProvider& svc);
	void set_time(int hour = 0, int minute = 0);
	void set_speed(int to_rate, int to_transition = 4096);
	[[nodiscard]] auto is_daytime() const -> bool { return increments.hours.get() >= 8 && increments.hours.get() < 19; }
	[[nodiscard]] auto is_nighttime() const -> bool { return increments.hours.get() >= 20 || increments.hours.get() < 7; }
	[[nodiscard]] auto is_twilight() const -> bool { return !is_daytime() && !is_nighttime(); }
	[[nodiscard]] auto is_transitioning() const -> bool { return transition.running(); }
	[[nodiscard]] auto num_cycles() const -> int { return static_cast<int>(TimeOfDay::END); }
	[[nodiscard]] auto get_time_of_day() const -> TimeOfDay { return is_daytime() ? TimeOfDay::day : is_nighttime() ? TimeOfDay::night : TimeOfDay::twilight; }
	[[nodiscard]] auto get_previous_time_of_day() const -> TimeOfDay { return previous_time_of_day; }
	[[nodiscard]] auto get_transition() const -> float { return transition.get_normalized(); }
	[[nodiscard]] auto get_hours() const -> int { return increments.hours.get(); }
	[[nodiscard]] auto get_minutes() const -> int { return increments.minutes.get(); }
	[[nodiscard]] auto get_rate() const -> int { return rate; }
	std::string get_string(bool military = true);

  private:
	struct {
		util::Circuit hours{24};
		util::Circuit minutes{60};
	} increments;
	int rate{};
	util::Cooldown transition{};
	TimeOfDay previous_time_of_day{};
};

} // namespace fornani