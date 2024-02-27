
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

	// this is a faulty approach. tick gets called way too many times per frame,
	// and the residue variable contains only the last iteration of the last 
	// object that used the function
	template <typename F>
	void tick(F fn) {

		ft = Tim{tick_rate};

		new_time = Clk::now();
		dt = std::chrono::duration_cast<Tim>(new_time - current_time);
		current_time = new_time;
		
		if (dt.count() > tick_limit.count()) { return; } // return for unexpected dt values, particularly during the beginning of the state

		accumulator = 2 * dt + residue;
		if (accumulator < ft) {
			std::printf("accumulator exit condition reached.\n");
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

	void start_frame() {
		++num_frames;
		total_seconds_passed += dt;
	}

	void end_frame() {
		ticks_per_frame = total_integrations / (num_frames * calls_per_frame);
		calls_per_frame = 0;
		calculate_fps();
	}

	void calculate_fps() {
		if (num_frames <= sample_size) {
			frame_list.push_back(dt);
		} else {
			frame_list.pop_front();
			frame_list.push_back(dt);
		}
		for (auto& frame : frame_list) { seconds_passed += frame; }

		fps = num_frames <= sample_size ? num_frames / seconds_passed.count() : sample_size / seconds_passed.count();
		seconds_passed = Tim::zero();
	}

	// fixed animation time step variables
	Clk::time_point current_time{Clk::now()};
	Clk::time_point new_time{Clk::now()};

	float tick_rate{0.016f};
	float tick_multiplier{16.f};

	static constexpr Tim tick_limit{0.2f};

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

	std::deque<Tim> frame_list{};
};

} // namespace util