
#pragma once

#include <chrono>
#include <cmath>
#include <list>
#include <random>
#include <vector>

namespace behavior {

int const DEFAULT_FRAMERATE = 8;
int const DEFAULT_DURATION = 8;
int const DEFAULT_NUM_BEHAVIORS = 1;
int const animation_multiplier = 32;

struct BehaviorParameters {
	BehaviorParameters() { set_params(); }
	BehaviorParameters(std::string id, int d, int f, bool t, bool r, bool nl, int lookup) : behavior_id(id), duration(d), framerate(f), transitional(t), restrictive(r), no_loop(nl), lookup_value(lookup) { set_params(); }
	~BehaviorParameters() {}
	void set_params() {
		current_frame = 0;
		anim_frame = framerate - 1;
		complete = false;
		started = true;
	}
	int framerate{};
	int current_frame{};
	int anim_frame{};
	int duration{};
	int lookup_value{};
	bool just_started{true};
	bool no_loop{};
	bool restrictive{};
	bool transitional{};
	bool complete = false;
	bool done = false;
	bool started = true;
	bool frame_trigger{};
	std::string behavior_id{};
};

class Behavior {

  public:
	Behavior() = default;
	Behavior(BehaviorParameters p) : params(p) { update(); }

	void refresh();
	bool start();
	void update();
	int get_frame();

	bool restricted();

	BehaviorParameters params{};
};

} // namespace behavior
