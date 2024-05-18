
#pragma once

#include <chrono>
#include <cmath>
#include <list>
#include <random>
#include <vector>
#include "../../utils/BitFlags.hpp"

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

enum class State {active, complete, just_started};

struct Animation {

	Parameters params{};
	std::string_view label{};

	void refresh();
	void start();
	void update();
	void end();
	void set_params(Parameters const new_params);
	int get_frame() const;

	bool active() const;
	bool complete() const;
	bool keyframe_over() const;
	[[nodiscard]] auto just_started() const -> bool { return flags.test(State::just_started); }

	int current_frame{};
	int counter{};
	int loop_counter{};

	util::BitFlags<State> flags{};

};

} // namespace anim
