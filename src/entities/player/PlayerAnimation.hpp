
#pragma once

#include <iostream>
#include <unordered_map>
#include <optional>
#include "../../utils/StateFunction.hpp"
#include "../animation/Animation.hpp"
#define PA_BIND(f) std::bind(&PlayerAnimation::f, this)

namespace player {

enum class AnimState { idle, turn, run, sprint, jumpsquat, rise, suspend, fall, stop, inspect, land, hurt, dash, wallslide };

// { lookup, duration, framerate, num_loops (-1 for infinite) }
inline anim::Parameters idle{20, 8, 28, -1};
inline anim::Parameters turn{33, 3, 18, 0};
inline anim::Parameters run{44, 4, 28, -1};
inline anim::Parameters sprint{10, 6, 16, -1};
inline anim::Parameters jumpsquat{61, 1, 16, 0};
inline anim::Parameters rise{54, 2, 22, -1};
inline anim::Parameters suspend{30, 3, 30, -1};
inline anim::Parameters fall{62, 4, 20, -1};
inline anim::Parameters stop{74, 2, 34, 0};
inline anim::Parameters land{56, 2, 28, 0};

inline anim::Parameters inspect{37, 2, 30, -1, true};
inline anim::Parameters hurt{76, 2, 22, 1};
inline anim::Parameters dash{40, 4, 22, 0};
inline anim::Parameters wallslide{66, 4, 28, -1};

class PlayerAnimation {

  public:
	PlayerAnimation() {
		state_function = state_function();
		animation.set_params(idle);
		animation.start();
		state.set(AnimState::idle);
	}

	anim::Animation animation{};
	util::BitFlags<AnimState> state{};

	void update();
	void start();
	int get_frame() const;

	// animation helpers
	bool not_jumping();

	float suspension_threshold{4.4f};
	float landed_threshold{0.004f};
	float stop_threshold{0.08f};
	float run_threshold{0.02f};

	fsm::StateFunction state_function = std::bind(&PlayerAnimation::update_idle, this);

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_sprint();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_rise();
	fsm::StateFunction update_suspend();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_land();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_dash();
	fsm::StateFunction update_wallslide();

	//std::optional<fsm::StateFunction> change_state(AnimState next, anim::Parameters params, fsm::StateFunction fn);

};

} // namespace player
