
#pragma once

#include <chrono>
#include <cmath>
#include <list>
#include <random>
#include <vector>

namespace anim {

int const DEFAULT_FRAMERATE = 8;
int const DEFAULT_DURATION = 8;
int const DEFAULT_NUM_BEHAVIORS = 1;
int const animation_multiplier = 32;

struct AnimationParameters {
	AnimationParameters() { set_params(); }
	AnimationParameters(int d, int f, bool nl) : duration(d), framerate(f), no_loop(nl) { set_params(); }
	~AnimationParameters() {}
	void set_params() {
		current_frame = 0;
		anim_frame = framerate - 1;
		started = true;
	}
	int framerate{};
	int current_frame{};
	int anim_frame{};
	int duration{};
	bool no_loop{};
	bool done{false};
	bool started{true};
	bool frame_trigger{};
};

class Animation {

	using Clock = std::chrono::steady_clock;
	using Time = std::chrono::duration<float>;

  public:
	Animation() = default;
	Animation(AnimationParameters p) : params(p) { update(); }

	void refresh();
	void start();
	void update();
	void end(bool cutoff);
	int get_frame();

	AnimationParameters params{};

	// fixed animation time step variables
	Time dt{0.001f};
	Clock::time_point current_time = Clock::now();
	Time accumulator{0.0f};
};

} // namespace anim
