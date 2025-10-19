
#pragma once

#include <chrono>
#include <deque>
#include <format>

#include "fornani/io/Logger.hpp"

namespace fornani::util {

class Stopwatch {
  public:
	using Clock = std::chrono::steady_clock;
	using Timepoint = std::chrono::time_point<std::chrono::steady_clock>;
	using DurationMilliseconds = std::chrono::duration<float, std::milli>;
	using DurationSeconds = std::chrono::duration<float>;

	void start();
	void resume();
	void stop();

	[[nodiscard]] auto get_time() const -> float { return stored_time_as_seconds().count(); }
	[[nodiscard]] auto get_elapsed_time() const -> DurationSeconds;
	[[nodiscard]] auto get_readout() const -> std::string { return std::format("{:.3f}", get_elapsed_time().count()); }
	[[nodiscard]] auto get_readout(float dt) const -> std::string { return std::format("{:.3f}", get_elapsed_time().count() * dt); }
	[[nodiscard]] auto get_final() const -> std::string { return std::format("{:.3f}", stored_time_as_seconds().count()); }
	[[nodiscard]] auto get_final(float dt) const -> std::string { return std::format("{:.3f}", stored_time_as_seconds().count() * dt); }
	[[nodiscard]] auto stored_time_as_seconds() const -> DurationSeconds { return std::chrono::duration_cast<DurationSeconds>(m_stored_time); }
	[[nodiscard]] auto as_seconds() const -> DurationSeconds { return std::chrono::duration_cast<DurationSeconds>(get_elapsed_time()); }
	void print_time(std::string_view message) const;
	void benchmark(std::string_view message);

  private:
	DurationMilliseconds m_stored_time{};
	DurationMilliseconds m_paused_time{};
	Timepoint m_start_time{Clock::now()};
	Timepoint m_end_time{};

	io::Logger m_logger{"stopwatch"};
};

} // namespace fornani::util
