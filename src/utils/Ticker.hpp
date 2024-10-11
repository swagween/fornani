
#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
#include <utility>
#include "Cooldown.hpp"
#include "BitFlags.hpp"

namespace util {

using Clk = std::chrono::steady_clock;
using Sec = std::chrono::duration<float>;
using Mil = std::chrono::milliseconds;

enum class TickerFlags { forced_slowdown, paused };
enum class Period { second, twenty_minutes };
struct PeriodicBool {
	PeriodicBool(Sec period) : period(period) {}
	Sec period{};
	Sec elapsed{};
	constexpr bool tick(Sec dt) {
		elapsed += dt;
		if (elapsed >= period) {
			elapsed = std::chrono::seconds{0};
			return true;
		}
		return false;
	}
};
class Ticker {
  public:
	template <typename F>
	void tick(F fn) {

		ft = Sec{tick_rate};

		if (!flags.test(TickerFlags::forced_slowdown)) {
			if (slowdown.running()) { dt_scalar -= 0.05f; }
			if (slowdown.is_complete()) { dt_scalar += 0.05f; }
			dt_scalar = std::clamp(dt_scalar, slowdown_rate, 1.f);
			if (freezeframe.running()) { dt_scalar = 0.1f; }
		}

		new_time = Clk::now();
		dt = std::chrono::duration_cast<Sec>(new_time - current_time);
		dt *= dt_scalar;
		current_time = new_time;

		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		accumulator = dt + residue;
		if (accumulator < ft) {
			residue += accumulator;
			accumulator = Sec::zero();
			return;
		}

		integrations = 0;
		while (accumulator >= ft) {
			new_tick_time = Clk::now();
			tick_dt = std::chrono::duration_cast<Sec>(new_tick_time - current_tick_time);
			tick_dt *= dt_scalar;
			current_tick_time = new_tick_time;
			std::cout << tick_dt.count() << "\n";
			second_ticker.tick(tick_dt) ? periods.set(Period::second) : periods.reset(Period::second);
			twenty_minute_ticker.tick(tick_dt) ? periods.set(Period::twenty_minutes) : periods.reset(Period::twenty_minutes);
			fn();
			accumulator -= ft;
			++integrations;
			++total_integrations;
			++ticks;
		}

		residue = accumulator;
		slowdown.update();
		freezeframe.update();
		accumulator = Sec::zero();
		++calls_per_frame;
	};

	void start_frame();
	void end_frame();
	void calculate_fps();
	void slow_down(int time);
	void freeze_frame(int time);
	void set_time(Sec time);
	void scale_dt();
	void reset_dt();
	void pause() { flags.set(TickerFlags::paused); }
	void unpause() { flags.reset(TickerFlags::paused); }
	[[nodiscard]] auto paused() const -> bool { return flags.test(TickerFlags::paused); }

	[[nodiscard]] auto global_tick_rate() const -> float { return ft.count() * tick_multiplier; }
	[[nodiscard]] auto every_x_frames(int const freq) const -> bool { return num_frames % freq == 0; }
	[[nodiscard]] auto every_x_ticks(int const freq) const -> bool { return ticks % freq == 0; }
	[[nodiscard]] auto every_second() const -> bool { return periods.test(Period::second); }
	[[nodiscard]] auto every_twenty_minutes() const -> bool { return periods.test(Period::twenty_minutes); }

	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};
	Clk::time_point current_tick_time{Clk::now()};
	Clk::time_point new_tick_time{Clk::now()};

	float tick_rate{0.005f};
	float tick_multiplier{24.f};
	float dt_scalar{1.f};

	static constexpr Sec tick_limit{0.8f};

	Sec ft{};
	Sec dt{};
	Sec tick_dt{};
	Sec accumulator{};
	Sec residue{};

	// for TPS and FPS calculations
	int integrations{};
	int ticks{};
	int calls_per_frame{};
	int sample_size{256};
	int num_frames{};
	float total_integrations{};
	float ticks_per_frame{};

	Sec seconds_passed{};
	Sec total_seconds_passed{};
	Sec in_game_seconds_passed{};
	Sec total_milliseconds_passed{};
	PeriodicBool second_ticker{std::chrono::seconds{1}};
	PeriodicBool twenty_minute_ticker{std::chrono::seconds{30}};
	float fps{60.f};

  private:
	std::deque<Sec> frame_list{};
	BitFlags<TickerFlags> flags{};
	BitFlags<Period> periods{};
	float slowdown_rate{0.2f};
	Cooldown slowdown{};
	Cooldown freezeframe{};
};

} // namespace util