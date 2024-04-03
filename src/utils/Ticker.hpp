
#pragma once

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <random>
#include <thread>
#include <utility>

namespace util {

using Clk = std::chrono::steady_clock;
using Tim = std::chrono::duration<float>;
using Mil = std::chrono::milliseconds;

class Ticker {
  public:
	template <typename F>
	void tick(F fn) {

		ft = Tim{tick_rate};

		new_time = Clk::now();
		dt = std::chrono::duration_cast<Tim>(new_time - current_time);
		current_time = new_time;

		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		accumulator = dt + residue;
		if (accumulator < ft) {
			residue += accumulator;
			accumulator = Tim::zero();
			return;
		}

		int integrations{0};
		while (accumulator >= ft) {

			fn();
			accumulator -= ft;
			++integrations;
		}

		residue = accumulator;

		total_integrations += integrations;
		accumulator = Tim::zero();
		++calls_per_frame;
	};

	float global_tick_rate();

	void start_frame();
	void end_frame();
	void calculate_fps();
	bool every_x_frames(int const freq);
	bool every_x_ticks(int const freq);
	bool every_x_milliseconds(int num_milliseconds);

	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};

	float tick_rate{0.005f};
	float tick_multiplier{24.f};

	static constexpr Tim tick_limit{0.8f};

	Tim ft{};
	Tim dt{};
	Tim accumulator{};
	Tim residue{};

	// for TPS and FPS calculations
	int integrations{};
	int calls_per_frame{};
	int sample_size{256};
	int num_frames{};
	float total_integrations{};
	float ticks_per_frame{};

	Tim seconds_passed{};
	Tim total_seconds_passed{};
	float fps{60.f};

  private:
	std::deque<Tim> frame_list{};
};

} // namespace util