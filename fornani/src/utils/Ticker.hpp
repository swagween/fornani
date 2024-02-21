
#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <random>
#include <utility>

namespace util {

using Clk = std::chrono::steady_clock;
using Tim = std::chrono::duration<float>;
using Mil = std::chrono::milliseconds;

struct Ticker {

	template <typename F>
	void tick(F fn) {

		ft = Tim{tick_rate};

		
		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		
		accumulator += dt;

		int integrations{0};
		while (accumulator >= ft) {

			fn();
			accumulator -= ft;
			++integrations;
		}

		++num_frames;
	};

	void start_frame() {
		new_time = Clk::now();
		dt = std::chrono::duration_cast<Tim>(new_time - current_time);
		current_time = new_time;
	}

	void end_frame() {
	}

	// fixed animation time step variables
	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};

	float tick_rate{0.0005f};
	float tick_multiplier{280.f};

	static constexpr Tim tick_limit{0.1f};

	Tim ft{};
	Tim dt{};
	Tim accumulator{};

	int num_frames{};
};

} // namespace util