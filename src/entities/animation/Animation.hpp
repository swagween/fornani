
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
	bool interruptible{};
};

enum class State { param_switch, keyframe, oneoff_complete };

struct Animation {

	Parameters params{};
	Parameters next_params{};
	std::string_view label{};

	void refresh();
	void start();
	void update();
	void set_params(Parameters const new_params, bool hard = true);
	void switch_params();
	void end() { frame.cancel(); }
	int get_frame() const;

	[[nodiscard]] auto active() const -> bool { return frame_timer.running(); }
	[[nodiscard]] auto synced() const -> bool { return params.lookup == next_params.lookup; }
	[[nodiscard]] auto complete() -> bool {
		auto ret = (params.repeat_last_frame ? (frame.get_count() == params.duration - 1) : frame.canceled());
		if (params.num_loops == 0) { ret = flags.consume(State::oneoff_complete); }
		if (frame.canceled()) { ret = true; }
		return ret;
	}
	[[nodiscard]] auto keyframe_over() -> bool { return flags.consume(State::keyframe); }
	[[nodiscard]] auto just_started() const -> bool { return global_counter.get_count() == 1; }

	util::Cooldown frame_timer{};
	util::Counter global_counter{};
	util::Counter loop{};
	util::Counter frame{};

	util::BitFlags<State> flags{};

};

} // namespace anim
