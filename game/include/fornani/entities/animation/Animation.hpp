
#pragma once

#include <chrono>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Counter.hpp"

namespace fornani::anim {

constexpr int DEFAULT_FRAMERATE = 8;
constexpr int DEFAULT_DURATION = 8;
constexpr int DEFAULT_NUM_BEHAVIORS = 1;
constexpr int animation_multiplier = 64;

struct Parameters {
	int lookup{};
	int duration{};
	int framerate{};
	int num_loops{};
	bool repeat_last_frame{};
	bool interruptible{};
};

enum class State : uint8_t { param_switch, keyframe, oneoff_complete };

struct Animation {

	Parameters params{};
	Parameters next_params{};
	std::string_view label{};

	void refresh();
	void start();
	void update();
	void set_params(Parameters new_params, bool hard = true);
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
	[[nodiscard]] auto keyframe_started() const -> bool { return frame_timer.get_cooldown() == params.framerate; }
	[[nodiscard]] auto just_started() const -> bool { return global_counter.get_count() == 1; }

	util::Cooldown frame_timer{};
	util::Counter global_counter{};
	util::Counter loop{};
	util::Counter frame{};

	util::BitFlags<State> flags{};

};

} // namespace fornani::anim
