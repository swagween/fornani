
#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <random>
#include <thread>
#include <utility>
#include "Cooldown.hpp"
#include "BitFlags.hpp"

namespace util {

using Clk = std::chrono::steady_clock;
using Tim = std::chrono::duration<float>;
using Mil = std::chrono::milliseconds;

enum class TickerFlags { forced_slowdown, paused };

class Ticker {
  public:
	template <typename F>
	void tick(F fn) {

		ft = Tim{tick_rate};

		if (!flags.test(TickerFlags::forced_slowdown)) {
			if (slowdown.running()) { dt_scalar -= 0.05f; }
			if (slowdown.is_complete()) { dt_scalar += 0.05f; }
			dt_scalar = std::clamp(dt_scalar, slowdown_rate, 1.f);
			if (freezeframe.running()) { dt_scalar = 0.1f; }
		}

		new_time = Clk::now();
		dt = std::chrono::duration_cast<Tim>(new_time - current_time);
		dt *= dt_scalar;
		current_time = new_time;

		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		accumulator = dt + residue;
		if (accumulator < ft) {
			residue += accumulator;
			accumulator = Tim::zero();
			return;
		}

		integrations = 0;
		while (accumulator >= ft) {
			if (!flags.test(TickerFlags::paused)) { fn(); }
			accumulator -= ft;
			++integrations;
			++total_integrations;
			++ticks;
		}

		residue = accumulator;
		slowdown.update();
		freezeframe.update();
		accumulator = Tim::zero();
		++calls_per_frame;
	};

	void start_frame();
	void end_frame();
	void calculate_fps();
	void slow_down(int time);
	void freeze_frame(int time);
	void scale_dt();
	void reset_dt();
	void pause() { flags.set(TickerFlags::paused); }
	void unpause() { flags.reset(TickerFlags::paused); }
	[[nodiscard]] auto paused() const -> bool { return flags.test(TickerFlags::paused); }

	[[nodiscard]] auto global_tick_rate() const -> float { return ft.count() * tick_multiplier; }
	[[nodiscard]] auto every_x_frames(int const freq) const -> bool { return num_frames % freq == 0; }
	[[nodiscard]] auto every_x_ticks(int const freq) const -> bool { return ticks % freq == 0; }
	[[nodiscard]] auto every_x_milliseconds(int num_milliseconds) const -> bool { return (int)std::floor(total_milliseconds_passed.count()) % num_milliseconds == 0; } // doesn't work

	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};

	float tick_rate{0.005f};
	float tick_multiplier{24.f};
	float dt_scalar{1.f};

	static constexpr Tim tick_limit{0.8f};

	Tim ft{};
	Tim dt{};
	Tim accumulator{};
	Tim residue{};

	// for TPS and FPS calculations
	int integrations{};
	int ticks{};
	int calls_per_frame{};
	int sample_size{256};
	int num_frames{};
	float total_integrations{};
	float ticks_per_frame{};

	Tim seconds_passed{};
	Tim total_seconds_passed{};
	Tim total_milliseconds_passed{};
	float fps{60.f};

  private:
	std::deque<Tim> frame_list{};
	BitFlags<TickerFlags> flags{};
	float slowdown_rate{0.2f};
	Cooldown slowdown{};
	Cooldown freezeframe{};
};

} // namespace util