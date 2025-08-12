
#include "fornani/entities/player/PlayerAnimation.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

constexpr auto rate{4};
// { lookup, duration, framerate, num_loops (-1 for infinite), repeat_last_frame, interruptible }

PlayerAnimation::PlayerAnimation(Player& plr)
	: m_player(&plr), m_params{{"idle", {20, 8, 7 * rate, -1, false, true}},
							   {"turn", {33, 3, 4 * rate, 0}},
							   {"sharp_turn", {16, 2, 5 * rate, 0}},
							   {"run", {44, 4, 6 * rate, -1}},
							   {"sprint", {10, 6, 4 * rate, -1}},
							   {"shield", {80, 3, 4 * rate, -1, true}},
							   {"between_push", {85, 1, 4 * rate, 0}},
							   {"push", {86, 4, 7 * rate, -1}},
							   {"rise", {40, 4, 6 * rate, 0}},
							   {"walljump", {40, 4, 6 * rate, 0}},
							   {"suspend", {30, 3, 7 * rate, -1}},
							   {"fall", {62, 4, 5 * rate, -1}},
							   {"stop", {74, 2, 4 * rate, 0}},
							   {"land", {56, 2, 4 * rate, 0}},
							   {"inspect", {37, 2, 7 * rate, -1, true}},
							   {"sit", {50, 4, 6 * rate, -1, true}},
							   {"hurt", {76, 2, 7 * rate, 0}},
							   {"dash", {0, 4, 4 * rate, 0}},
							   {"dash_up", {120, 4, 4 * rate, 0}},
							   {"dash_down", {124, 4, 4 * rate, 0}},
							   {"wallslide", {66, 4, 7 * rate, -1}},
							   {"die", {76, 4, 8 * rate, -1, true}},
							   {"backflip", {90, 6, 5 * rate, 0}},
							   {"slide", {96, 4, 4 * rate, -1}},
							   {"get_up", {57, 1, 5 * rate, 0}},
							   {"roll", {100, 4, 5 * rate, 0}},
							   {"shoot", {104, 3, 8 * rate, 0}},
							   {"sleep", {4, 4, 8 * rate, -1, true}},
							   {"wake_up", {8, 2, 8 * rate, 0}},
							   {"crouch", {110, 5, 4 * rate, -1, true}},
							   {"crawl", {114, 4, 6 * rate, -1}}},
	  state_function{std::bind(&PlayerAnimation::update_idle, this)}, m_buffer{16} {
	state_function = state_function();
	animation.set_params(get_params("idle"));
	animation.start();
	request(AnimState::idle);
}

void PlayerAnimation::update() {
	animation.update();
	state_function = state_function();
	if (m_player->is_dead()) { request(AnimState::die); }
	if (m_buffer.is_complete()) { m_requested = {}; }
	m_buffer.update();
}

void PlayerAnimation::start() { animation.start(); }

bool PlayerAnimation::stepped() const { return (animation.get_frame() == 44 || animation.get_frame() == 46 || animation.get_frame() == 10 || animation.get_frame() == 13 || animation.get_frame() == 16) && animation.keyframe_started(); }

fsm::StateFunction PlayerAnimation::update_idle() {
	animation.label = "idle";
	m_actual = AnimState::idle;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (animation.just_started()) {
		idle_timer.start();
		m_player->cooldowns.push.start();
	}
	idle_timer.update();
	if (idle_timer.get_count() > timers.sit) { request(AnimState::sit); }
	if (change_state(AnimState::sit, get_params("sit"))) {
		idle_timer.cancel();
		return PA_BIND(update_sit);
	}
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::shoot, get_params("shoot"))) { return PA_BIND(update_shoot); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_sprint() {
	animation.label = "sprint";
	m_actual = AnimState::sprint;
	m_player->controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (!m_player->firing_weapon()) {
		if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::slide, get_params("slide"), true)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
		if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
		if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	}
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::stop, get_params("stop"))) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_shield() {
	animation.label = "shield";
	m_actual = AnimState::shield;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }

	return PA_BIND(update_shield);
}

fsm::StateFunction PlayerAnimation::update_between_push() {
	animation.label = "between push";
	m_actual = AnimState::between_push;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (animation.complete()) {
		if (change_state(AnimState::push, get_params("push"))) { return PA_BIND(update_push); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		request(AnimState::idle);
		if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	}

	return PA_BIND(update_between_push);
}

fsm::StateFunction PlayerAnimation::update_push() {
	animation.label = "push";
	m_actual = AnimState::push;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (!was_requested(AnimState::push)) {
		animation.set_params(get_params("between_push"));
		return PA_BIND(update_between_push);
	}

	return PA_BIND(update_push);
}

fsm::StateFunction PlayerAnimation::update_run() {
	animation.label = "run";
	m_actual = AnimState::run;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (!m_player->firing_weapon()) {
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
		if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
		if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	}
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::stop, get_params("stop"))) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_turn() {
	animation.label = "turn";
	m_actual = AnimState::turn;
	if (change_state(AnimState::die, get_params("die"), true)) {
		triggers.set(AnimTriggers::flip);
		return PA_BIND(update_die);
	}
	if (change_state(AnimState::rise, get_params("rise"))) {
		triggers.set(AnimTriggers::flip);
		return PA_BIND(update_rise);
	}
	if (animation.complete()) {
		triggers.set(AnimTriggers::flip);
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
		if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
		if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}

	return PA_BIND(update_turn);
}

fsm::StateFunction PlayerAnimation::update_sharp_turn() {
	animation.label = "sharp_turn";
	m_actual = AnimState::sharp_turn;
	if (change_state(AnimState::die, get_params("die"), true)) {
		triggers.set(AnimTriggers::flip);
		return PA_BIND(update_die);
	}
	if (change_state(AnimState::rise, get_params("rise"))) {
		triggers.set(AnimTriggers::flip);
		return PA_BIND(update_rise);
	}
	if (animation.complete()) {
		triggers.set(AnimTriggers::flip);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}

	return PA_BIND(update_sharp_turn);
}

fsm::StateFunction PlayerAnimation::update_rise() {
	animation.label = "rise";
	m_actual = AnimState::rise;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		if (m_player->grounded()) {
			animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
		animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_suspend() {
	animation.label = "suspend";
	m_actual = AnimState::suspend;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_fall() {
	animation.label = "fall";
	m_actual = AnimState::fall;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (m_player->grounded()) {
		animation.set_params(get_params("land"));
		return PA_BIND(update_land);
	}

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_stop() {
	animation.label = "stop";
	m_actual = AnimState::stop;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }

	if (animation.complete()) {
		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }

	return PA_BIND(update_stop);
}

fsm::StateFunction PlayerAnimation::update_inspect() {
	animation.label = "inspect";
	m_actual = AnimState::inspect;
	m_player->controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::sleep, get_params("sleep"))) { return PA_BIND(update_sleep); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
		if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	}
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }

	return PA_BIND(update_inspect);
}

fsm::StateFunction PlayerAnimation::update_sit() {
	animation.label = "sit";
	m_actual = AnimState::sit;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (animation.complete()) {
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	}
	if (m_player->arsenal && m_player->hotbar) {
		if (m_player->equipped_weapon().cooling_down()) {
			animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
	}
	return PA_BIND(update_sit);
}

fsm::StateFunction PlayerAnimation::update_land() {
	animation.label = "land";
	m_actual = AnimState::land;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (animation.complete()) {
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
		if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_land);
}

fsm::StateFunction PlayerAnimation::update_hurt() {
	animation.label = "hurt";
	m_actual = AnimState::hurt;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
	if (animation.complete()) {
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
		if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_dash() {
	animation.label = "dash";
	m_actual = AnimState::dash;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (animation.complete()) {
		m_player->flags.state.set(State::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash);
}

fsm::StateFunction PlayerAnimation::update_dash_up() {
	animation.label = "dash up";
	m_actual = AnimState::dash_up;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (animation.complete()) {
		m_player->flags.state.set(State::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash_up);
}

fsm::StateFunction PlayerAnimation::update_dash_down() {
	animation.label = "dash down";
	m_actual = AnimState::dash_down;
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (animation.complete()) {
		m_player->flags.state.set(State::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash_down);
}

fsm::StateFunction PlayerAnimation::update_wallslide() {
	animation.label = "wallslide";
	m_actual = AnimState::wallslide;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::walljump, get_params("walljump"), true)) { return PA_BIND(update_walljump); }
	if (change_state(AnimState::rise, get_params("walljump"), true)) { return PA_BIND(update_walljump); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

	return PA_BIND(update_wallslide);
}

fsm::StateFunction PlayerAnimation::update_walljump() {
	animation.label = "walljump";
	m_actual = AnimState::walljump;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}
	return PA_BIND(update_walljump);
}

fsm::StateFunction PlayerAnimation::update_die() {
	animation.label = "die";
	m_actual = AnimState::die;
	if (animation.just_started()) {
		m_player->m_services->music_player.stop();
		post_death.start();
		triggers.reset(AnimTriggers::end_death);
		m_player->m_services->state_controller.actions.set(automa::Actions::death_mode); // set here, reset on map load
	}
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	m_player->collider.collision_depths = {};
	post_death.update();
	if (!m_player->m_services->death_mode()) {
		m_player->collider.collision_depths = util::CollisionDepth();
		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	if (post_death.is_complete()) {
		m_player->collider.collision_depths = util::CollisionDepth();
		if (change_state(AnimState::idle, get_params("idle"), true)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::run, get_params("run"), true)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, get_params("sprint"), true)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::rise, get_params("rise"), true)) { return PA_BIND(update_rise); }
		triggers.set(AnimTriggers::end_death);
	}
	return PA_BIND(update_die);
}

fsm::StateFunction PlayerAnimation::update_backflip() {
	animation.label = "backflip";
	m_actual = AnimState::backflip;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}
	return PA_BIND(update_backflip);
}

fsm::StateFunction PlayerAnimation::update_slide() {
	animation.label = "slide";
	m_actual = AnimState::slide;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }

	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (!m_player->controller.is_sliding()) {
		request(AnimState::get_up);
		if (change_state(AnimState::get_up, get_params("get_up"))) { return PA_BIND(update_get_up); }
	}

	return PA_BIND(update_slide);
}

fsm::StateFunction PlayerAnimation::update_get_up() {
	animation.label = "get_up";
	m_actual = AnimState::get_up;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }

	if (animation.complete()) {
		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }

	return PA_BIND(update_get_up);
}

fsm::StateFunction PlayerAnimation::update_roll() {
	animation.label = "roll";
	m_actual = AnimState::roll;
	auto& controller = m_player->controller;
	controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::inspect, get_params("inspect"))) {
		m_player->collider.physics.stop_x();
		return PA_BIND(update_inspect);
	}
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }

	if (animation.complete()) {
		if (controller.is_sliding() && !m_player->firing_weapon()) {
			animation.set_params(get_params("slide"));
			return PA_BIND(update_slide);
		} else {
			if (controller.sprinting()) {
				animation.set_params(get_params("sprint"));
				return PA_BIND(update_sprint);
			}
			animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
	}
	return PA_BIND(update_roll);
}

fsm::StateFunction PlayerAnimation::update_shoot() {
	animation.label = "shoot";
	m_actual = AnimState::shoot;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::sprint, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::shoot, get_params("shoot"))) { return PA_BIND(update_shoot); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::stop, get_params("stop"))) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (animation.complete()) {
		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_shoot);
}

fsm::StateFunction player::PlayerAnimation::update_sleep() {
	animation.label = "sleep";
	m_actual = AnimState::sleep;
	m_player->flags.state.reset(State::show_weapon);
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	if (change_state(AnimState::wake_up, get_params("wake_up"), true)) { return PA_BIND(update_wake_up); }
	return PA_BIND(update_sleep);
}

fsm::StateFunction player::PlayerAnimation::update_wake_up() {
	animation.label = "wake_up";
	m_actual = AnimState::wake_up;
	m_player->flags.state.reset(State::show_weapon);
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	if (animation.complete()) {
		m_player->flags.state.set(State::show_weapon);
		m_player->controller.unrestrict();
		animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_wake_up);
}

fsm::StateFunction player::PlayerAnimation::update_crouch() {
	animation.label = "crouch";
	m_actual = AnimState::crouch;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (animation.complete()) {
		if (change_state(AnimState::crawl, get_params("crawl"), true)) { return PA_BIND(update_crawl); }
	}
	if (!m_player->controller.is_crouching()) {
		request(AnimState::get_up);
		if (change_state(AnimState::get_up, get_params("get_up"), true)) { return PA_BIND(update_get_up); }
	}
	return PA_BIND(update_crouch);
}

fsm::StateFunction player::PlayerAnimation::update_crawl() {
	animation.label = "crawl";
	m_actual = AnimState::crawl;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::crouch, get_params("crouch"), true)) {
		animation.set_frame(4);
		return PA_BIND(update_crouch);
	}
	if (!m_player->controller.is_crouching()) {
		request(AnimState::get_up);
		if (change_state(AnimState::get_up, get_params("get_up"), true)) { return PA_BIND(update_get_up); }
	}
	return PA_BIND(update_crawl);
}

bool PlayerAnimation::change_state(AnimState next, anim::Parameters params, bool hard) {
	if (m_requested.test(next)) {
		animation.set_params(params, hard);
		return true;
	}
	return false;
}

void player::PlayerAnimation::request(AnimState to_state) {
	m_requested = {};
	m_requested.set(to_state);
	m_buffer.start();
}

void player::PlayerAnimation::force(AnimState to_state, std::string_view key) {
	m_requested = {};
	m_requested.set(to_state);
	change_state(to_state, get_params(key.data()), true);
	m_buffer.start();
}

anim::Parameters const& player::PlayerAnimation::get_params(std::string const& key) { return m_params.contains(key) ? m_params.at(key) : m_params.at("idle"); }

} // namespace fornani::player
