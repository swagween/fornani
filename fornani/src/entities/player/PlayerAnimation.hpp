
#pragma once

#include <iostream>
#include <unordered_map>
#include <optional>
#include "../../utils/StateFunction.hpp"
#include "../behavior/Animation.hpp"
#define BIND(f) std::bind(&PlayerAnimation::f, this)

namespace player {

enum class AnimState { idle, turn, run, jumpsquat, rise, suspend, fall, stop, inspect, land };

inline anim::Parameters idle{20, 8, 18, false};
inline anim::Parameters turn{33, 3, 8, true};
inline anim::Parameters run{44, 4, 18, false};
inline anim::Parameters jumpsquat{61, 1, 12, true};
inline anim::Parameters rise{54, 2, 12, false};
inline anim::Parameters suspend{30, 3, 20, false};
inline anim::Parameters fall{62, 4, 10, false};
inline anim::Parameters stop{75, 1, 24, true};
inline anim::Parameters inspect{37, 2, 12, false, true};
inline anim::Parameters land{56, 2, 18, true};

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
	float stop_threshold{1.8f};
	float run_threshold{0.02f};

	fsm::StateFunction state_function = std::bind(&PlayerAnimation::update_idle, this);

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_rise();
	fsm::StateFunction update_suspend();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_land();

	//std::optional<fsm::StateFunction> change_state(AnimState next, anim::Parameters params, fsm::StateFunction fn);

};

} // namespace player
