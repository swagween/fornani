
#pragma once

#include <chrono>
#include <deque>
#include <format>

#include "fornani/io/Logger.hpp"

namespace fornani::util {

class Stopwatch {

  public:
	using Clk = std::chrono::steady_clock;
	using Tpt = std::chrono::time_point<std::chrono::steady_clock>;
	using Time = std::chrono::duration<float, std::milli>;
	using TimeSec = std::chrono::duration<float>;

	void start() { start_time = Clk::now(); };
	void stop() {
		elapsed_time = Clk::now() - start_time;
		seconds = std::chrono::duration_cast<TimeSec>(elapsed_time);
		snapshot.history.push_back(elapsed_time.count());
		calculate_snapshot();
	};
	[[nodiscard]] float seconds_passed() const { return seconds.count(); }
	[[nodiscard]] float get_snapshot() const { return snapshot.average_elapsed_ms; }
	void print_time() const { NANI_LOG_DEBUG(m_logger, "Elapsed Time: {:.5f} seconds", seconds.count()); }

	Time elapsed_time{};
	Tpt start_time = Clk::now();

  private:
	TimeSec seconds{};
	struct {
		std::deque<float> history{};
		float average_elapsed_ms{};
		int sample_size{512};
		int interval{128};
		int current_tick{};
	} snapshot{};
	void calculate_snapshot() {
		if (snapshot.history.size() >= snapshot.sample_size) { snapshot.history.pop_front(); }
		float average{};
		float running_total{};
		for (auto& sample : snapshot.history) { running_total += sample; }
		average = running_total / snapshot.history.size();
		++snapshot.current_tick;
		if (snapshot.current_tick % snapshot.interval == 0) { snapshot.average_elapsed_ms = average; }
	}

	io::Logger m_logger{"stopwatch"};
};

} // namespace fornani::util