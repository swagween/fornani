
#pragma once

#include <chrono>
#include <memory>
#include <random>

namespace util {

class Clock {

  public:
	using Clk = std::chrono::steady_clock;
	using Tpt = std::chrono::time_point<std::chrono::steady_clock>;
	using Time = std::chrono::duration<float>;

	Clock() = default;

	void tick() {
		auto new_time = Clk::now();
		auto frame_time = Time{new_time - current_time};
		current_time = new_time;
		accumulator += frame_time.count();
		elapsed_time += frame_time;
		seconds += elapsed_time.count();

		if (elapsed_time.count() > dt.count()) { elapsed_time = Time::zero(); }

		FPS = static_cast<float>(frame) / accumulator;

		tick_rate = Time(rate);

		++frame;
	}

	bool every_x_frames(int const freq) { return frame % freq == 0; }

	bool every_x_seconds(int const num_seconds) {}

	bool every_x_milliseconds(int num_milliseconds) {}

	int seconds_int() { return static_cast<int>(std::floor(seconds)); }

	Time elapsed_time{};
	Tpt current_time = Clk::now();
	float seconds{0.0f};
	int frame{0};

	Time dt{0.001f};
	float accumulator{0.0f};

	Time tick_rate{0.001f};
	float const tick_multiplier{0.25f};
	float const frame_limit{0.032f};

	float FPS{60.0f}; // assume 60 to begin with
	float rate{0.001f};
};

} // namespace util