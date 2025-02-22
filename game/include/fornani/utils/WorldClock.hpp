
#pragma once

#include <string>
#include "Circuit.hpp"
#include "Cooldown.hpp"

#include <cstdint>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {

enum class TimeOfDay : std::uint8_t { day, twilight, night, END };
enum class ClockMode { standard, military };

class WorldClock {
  public:
	WorldClock();
	void update(automa::ServiceProvider& svc);
	void set_time(int hour = 0, int minute = 0);
	void set_speed(int to_rate, int to_transition = 4096);
	void toggle_military_time();
	[[nodiscard]] auto is_military() const -> bool { return m_mode == ClockMode::military; }
	[[nodiscard]] auto get_normalized_time() const -> float { return static_cast<float>(get_hours() * 60 + get_minutes()) / 1440.f; }
	[[nodiscard]] auto is_daytime() const -> bool { return increments.hours.get() >= 8 && increments.hours.get() < 19; }
	[[nodiscard]] auto is_nighttime() const -> bool { return increments.hours.get() >= 20 || increments.hours.get() < 7; }
	[[nodiscard]] auto is_twilight() const -> bool { return !is_daytime() && !is_nighttime(); }
	[[nodiscard]] auto is_transitioning() const -> bool { return transition.running(); }
	[[nodiscard]] static auto num_cycles() -> int { return static_cast<int>(TimeOfDay::END); }
	[[nodiscard]] auto get_time_of_day() const -> TimeOfDay { return is_daytime() ? TimeOfDay::day : is_nighttime() ? TimeOfDay::night : TimeOfDay::twilight; }
	[[nodiscard]] auto get_previous_time_of_day() const -> TimeOfDay { return previous_time_of_day; }
	[[nodiscard]] auto get_transition() const -> float { return transition.get_normalized(); }
	[[nodiscard]] auto get_hours() const -> int { return increments.hours.get(); }
	[[nodiscard]] auto get_minutes() const -> int { return increments.minutes.get(); }
	[[nodiscard]] auto get_rate() const -> int { return rate; }
	std::string get_string();
	std::string get_hours_string();
	std::string get_minutes_string();

  private:
	ClockMode m_mode{};
	struct {
		util::Circuit hours{24};
		util::Circuit minutes{60};
	} increments;
	int rate{};
	util::Cooldown transition{};
	TimeOfDay previous_time_of_day{};
};

} // namespace fornani
