
#pragma once

#include <chrono>
#include <cmath>
#include <list>
#include <random>
#include <vector>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../utils/Counter.hpp"

namespace anim {

int const DEFAULT_FRAMERATE = 8;
int const DEFAULT_DURATION = 8;
int const DEFAULT_NUM_BEHAVIORS = 1;
int const animation_multiplier = 64;

struct Parameters {
	int lookup{};
	int duration{};
	int framerate{};
	int num_loops{};
	bool repeat_last_frame{};
	bool interruptible{true};
};

enum class State {active, complete, just_started, param_switch};

struct Animation {

	Parameters params{};
	Parameters next_params{};
	std::string_view label{};

	void refresh();
	void start();
	void update();
	void end();
	void set_params(Parameters const new_params);
	void switch_params();
	int get_frame() const;

	bool active() const;
	bool complete() const;
	bool keyframe_over() const;
	[[nodiscard]] auto just_started() const -> bool { return global_counter.get_count() == 1; }

	util::Cooldown frame_timer{};
	util::Counter global_counter{};
	util::Counter loop{};
	util::Counter frame{};

	util::BitFlags<State> flags{};

};

} // namespace anim
