
#pragma once

#include <iostream>
#include <optional>
#include <unordered_map>
#include "../../utils/StateFunction.hpp"
#include "../animation/Animation.hpp"
#define PA_BIND(f) std::bind(&PlayerAnimation::f, this)

namespace player {

enum class AnimState { idle, turn, sharp_turn, run, sprint, shield, jumpsquat, rise, suspend, fall, stop, inspect, sit, land, hurt, dash, wallslide };
int const rate{5};
// { lookup, duration, framerate, num_loops (-1 for infinite) }
inline anim::Parameters idle{20, 8, 7 * rate, -1};
inline anim::Parameters turn{33, 3, 4 * rate, 0};
inline anim::Parameters sharp_turn{16, 2, 4 * rate, 0};
inline anim::Parameters run{44, 4, 7 * rate, -1};
inline anim::Parameters sprint{10, 6, 4 * rate, -1};
inline anim::Parameters shield{80, 3, 4 * rate, -1, true};
inline anim::Parameters jumpsquat{61, 1, 4 * rate, 0};
inline anim::Parameters rise{54, 2, 5 * rate, -1};
inline anim::Parameters suspend{30, 3, 7 * rate, -1};
inline anim::Parameters fall{62, 4, 5 * rate, -1};
inline anim::Parameters stop{74, 2, 8 * rate, 0};
inline anim::Parameters land{56, 2, 7 * rate, 0};
inline anim::Parameters inspect{37, 2, 7 * rate, -1, true};
inline anim::Parameters sit{50, 4, 6 * rate, -1, true};
inline anim::Parameters hurt{76, 2, 7 * rate, 0};
inline anim::Parameters dash{40, 4, 5 * rate, 0};
inline anim::Parameters wallslide{66, 4, 7 * rate, -1};

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

	fsm::StateFunction state_function = std::bind(&PlayerAnimation::update_idle, this);

	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_sharp_turn();
	fsm::StateFunction update_sprint();
	fsm::StateFunction update_shield();
	fsm::StateFunction update_run();
	fsm::StateFunction update_jumpsquat();
	fsm::StateFunction update_rise();
	fsm::StateFunction update_suspend();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_stop();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_sit();
	fsm::StateFunction update_land();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_dash();
	fsm::StateFunction update_wallslide();

	bool change_state(AnimState next, anim::Parameters params);

  private:
	struct {
		int sit{2400};
	} timers{};
};

} // namespace player
