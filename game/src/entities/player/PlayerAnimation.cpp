
#include "fornani/entities/player/PlayerAnimation.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

constexpr auto rate{4};
// { lookup, duration, framerate, num_loops (-1 for infinite), repeat_last_frame, interruptible }

PlayerAnimation::PlayerAnimation(Player& plr) : m_player(&plr), state_function{std::bind(&PlayerAnimation::update_idle, this)}, m_buffer{16}, m_sleep_timer{512} {
	plr.m_params = {{"idle", {20, 8, 7 * rate, -1, false, true}},
					{"turn", {33, 3, 4 * rate, 0}},
					{"sharp_turn", {16, 2, 5 * rate, 0}},
					{"run", {44, 4, 6 * rate, -1}},
					{"sprint", {10, 6, 4 * rate, -1}},
					{"shield", {80, 3, 4 * rate, -1, true}},
					{"between_push", {85, 1, 2 * rate, 0}},
					{"push", {86, 4, 5 * rate, -1}},
					{"rise", {40, 4, 6 * rate, 0}},
					{"walljump", {90, 6, 6 * rate, 0}},
					{"suspend", {30, 3, 7 * rate, -1}},
					{"fall", {62, 4, 5 * rate, -1}},
					{"stop", {74, 2, 6 * rate, 0}},
					{"land", {56, 2, 5 * rate, 0, false, true}},
					{"inspect", {37, 2, 7 * rate, -1, true}},
					{"dash_kick", {108, 2, 6 * rate, 0}},
					{"sit", {50, 4, 6 * rate, -1, true}},
					{"hurt", {76, 2, 7 * rate, 0}},
					{"dash", {0, 4, 4 * rate, 0}},
					{"dash_up", {120, 4, 4 * rate, 0}},
					{"dash_down", {124, 4, 4 * rate, 0}},
					{"wallslide", {66, 4, 7 * rate, -1}},
					{"die", {76, 4, 8 * rate, -1, true}},
					{"drown", {149, 4, 8 * rate, -1, true}},
					{"backflip", {90, 6, 7 * rate, 0}},
					{"slide", {96, 4, 4 * rate, -1}},
					{"get_up", {57, 1, 5 * rate, 0}},
					{"roll", {100, 4, 5 * rate, 0}},
					{"shoot", {104, 3, 8 * rate, 0}},
					{"sleep", {4, 4, 8 * rate, -1, true}},
					{"wake_up", {8, 2, 8 * rate, 0}},
					{"crouch", {110, 5, 4 * rate, -1, true}},
					{"crawl", {114, 4, 6 * rate, -1}},
					{"turn_slide", {130, 7, 4 * rate, 0}},
					{"dive", {138, 6, 6 * rate, 0}},
					{"swim", {145, 4, 7 * rate, -1}},
					{"slow_walk", {44, 4, 8 * rate, -1}}};

	state_function = state_function();
	m_player->animation.set_params(get_params("idle"));
	m_player->animation.start();
	request(AnimState::idle);
}

int PlayerAnimation::get_frame() const { return m_player->animation.get_frame(); }

void PlayerAnimation::update() {
	state_function = state_function();
	if (m_player->is_dead()) { request(AnimState::die); }
	m_buffer.update();
	m_sleep_timer.update();
}

void PlayerAnimation::start() { m_player->animation.start(); }

bool PlayerAnimation::stepped() const {
	return (m_player->animation.get_frame() == 44 || m_player->animation.get_frame() == 46 || m_player->animation.get_frame() == 10 || m_player->animation.get_frame() == 13 || m_player->animation.get_frame() == 16) &&
		   m_player->animation.keyframe_started();
}

void PlayerAnimation::set_sleep_timer() { m_sleep_timer.start(); }

fsm::StateFunction PlayerAnimation::update_idle() {
	m_player->animation.label = "idle";
	p_state.actual = AnimState::idle;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (m_player->animation.just_started()) {
		idle_timer.start();
		m_player->cooldowns.push.start();
	}
	idle_timer.update();
	if (idle_timer.get_count() > timers.sit) { request(AnimState::sit); }
	if (change_state(AnimState::sit, get_params("sit"))) {
		idle_timer.cancel();
		return PA_BIND(update_sit);
	}
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
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
	m_player->animation.label = "sprint";
	p_state.actual = AnimState::sprint;
	m_player->controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (change_state(AnimState::turn_slide, get_params("turn_slide"))) { return PA_BIND(update_turn_slide); }
	if (!m_player->firing_weapon()) {
		if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
		if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
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
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
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
	m_player->animation.label = "shield";
	p_state.actual = AnimState::shield;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
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
	m_player->animation.label = "between push";
	p_state.actual = AnimState::between_push;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (m_player->animation.complete()) {
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
	m_player->animation.label = "push";
	p_state.actual = AnimState::push;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (!was_requested(AnimState::push)) {
		m_player->animation.set_params(get_params("between_push"));
		return PA_BIND(update_between_push);
	}

	return PA_BIND(update_push);
}

fsm::StateFunction PlayerAnimation::update_run() {
	m_player->animation.label = "run";
	p_state.actual = AnimState::run;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
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
	m_player->animation.label = "turn";
	p_state.actual = AnimState::turn;
	if (change_state(AnimState::die, get_params("die"), true)) {
		m_player->request_flip();
		return PA_BIND(update_die);
	}
	if (change_state(AnimState::rise, get_params("rise"))) {
		m_player->request_flip();
		return PA_BIND(update_rise);
	}
	if (change_state(AnimState::slide, get_params("slide"), true)) { return PA_BIND(update_slide); }
	if (m_player->animation.complete()) {
		m_player->request_flip();
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
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

		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}

	return PA_BIND(update_turn);
}

fsm::StateFunction PlayerAnimation::update_sharp_turn() {
	m_player->animation.label = "sharp_turn";
	p_state.actual = AnimState::sharp_turn;
	if (change_state(AnimState::die, get_params("die"), true)) {
		m_player->request_flip();
		return PA_BIND(update_die);
	}
	if (change_state(AnimState::rise, get_params("rise"))) {
		m_player->request_flip();
		return PA_BIND(update_rise);
	}
	if (change_state(AnimState::slide, get_params("slide"))) {
		m_player->request_flip();
		return PA_BIND(update_slide);
	}
	if (m_player->animation.complete()) {
		m_player->request_flip();
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}

	return PA_BIND(update_sharp_turn);
}

fsm::StateFunction PlayerAnimation::update_rise() {
	m_player->animation.label = "rise";
	p_state.actual = AnimState::rise;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (m_player->animation.complete()) {
		if (m_player->has_collider()) {
			if (m_player->grounded()) {
				m_player->animation.set_params(get_params("idle"));
				return PA_BIND(update_idle);
			}
		}
		m_player->animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_suspend() {
	m_player->animation.label = "suspend";
	p_state.actual = AnimState::suspend;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
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
	m_player->animation.label = "fall";
	p_state.actual = AnimState::fall;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (change_state(AnimState::turn_slide, get_params("turn_slide"))) { return PA_BIND(update_turn_slide); }
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
	if (m_player->has_collider()) {
		if (m_player->grounded()) {
			m_player->animation.set_params(get_params("land"));
			return PA_BIND(update_land);
		}
	}

	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_stop() {
	m_player->animation.label = "stop";
	p_state.actual = AnimState::stop;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }

	if (m_player->animation.complete()) {
		m_player->animation.set_params(get_params("idle"));
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
	m_player->animation.label = "inspect";
	p_state.actual = AnimState::inspect;
	m_player->controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::sleep, get_params("sleep"))) { return PA_BIND(update_sleep); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (m_player->animation.complete()) {
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
	m_player->animation.label = "sit";
	p_state.actual = AnimState::sit;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
	if (m_player->animation.complete()) {
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
	}
	if (m_player->arsenal && m_player->hotbar) {
		if (m_player->equipped_weapon().cooling_down()) {
			m_player->animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
	}
	return PA_BIND(update_sit);
}

fsm::StateFunction PlayerAnimation::update_land() {
	m_player->animation.label = "land";
	p_state.actual = AnimState::land;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::inspect, get_params("inspect"))) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (change_state(AnimState::turn_slide, get_params("turn_slide"))) { return PA_BIND(update_turn_slide); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (m_player->animation.complete()) {
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::crouch, get_params("crouch"))) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::crawl, get_params("crouch"), true)) { return PA_BIND(update_crouch); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
		request(AnimState::idle);
		if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	}
	return PA_BIND(update_land);
}

fsm::StateFunction PlayerAnimation::update_hurt() {
	m_player->animation.label = "hurt";
	p_state.actual = AnimState::hurt;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
	if (m_player->animation.complete()) {
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
		request(AnimState::idle);
		if (change_state(AnimState::idle, get_params("idle"))) { return PA_BIND(update_idle); }
	}

	return PA_BIND(update_hurt);
}

fsm::StateFunction PlayerAnimation::update_dash() {
	m_player->animation.label = "dash";
	p_state.actual = AnimState::dash;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::dash_kick, get_params("dash_kick"))) { return PA_BIND(update_dash_kick); }
	if (change_state(AnimState::turn_slide, get_params("turn_slide"))) { return PA_BIND(update_turn_slide); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (m_player->animation.complete()) {
		m_player->set_flag(PlayerFlags::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash);
}

fsm::StateFunction PlayerAnimation::update_dash_up() {
	m_player->animation.label = "dash up";
	p_state.actual = AnimState::dash_up;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::dash_kick, get_params("dash_kick"))) { return PA_BIND(update_dash_kick); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (m_player->animation.complete()) {
		m_player->set_flag(PlayerFlags::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash_up);
}

fsm::StateFunction PlayerAnimation::update_dash_down() {
	m_player->animation.label = "dash down";
	p_state.actual = AnimState::dash_down;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::dash_kick, get_params("dash_kick"))) { return PA_BIND(update_dash_kick); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::backflip, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::roll, get_params("roll"))) { return PA_BIND(update_roll); }
	if (m_player->animation.complete()) {
		m_player->set_flag(PlayerFlags::show_weapon);
		if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
		if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, get_params("fall"))) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, get_params("land"))) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, get_params("shield"))) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }

		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash_down);
}

fsm::StateFunction PlayerAnimation::update_wallslide() {
	m_player->animation.label = "wallslide";
	p_state.actual = AnimState::wallslide;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::walljump, get_params("walljump"), true)) { return PA_BIND(update_walljump); }
	if (change_state(AnimState::rise, get_params("walljump"), true)) { return PA_BIND(update_walljump); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::swim, get_params("swim"))) { return PA_BIND(update_swim); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
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
	m_player->animation.label = "walljump";
	p_state.actual = AnimState::walljump;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (m_player->animation.complete()) {
		m_player->animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}
	return PA_BIND(update_walljump);
}

fsm::StateFunction PlayerAnimation::update_die() {
	m_player->animation.label = "die";
	p_state.actual = AnimState::die;
	if (m_player->animation.just_started()) {
		m_player->m_services->music_player.stop();
		post_death.start();
		triggers.reset(AnimTriggers::end_death);
	}
	if (m_player->has_collider()) {
		m_player->animation.linger_on_frame(1, !m_player->get_collider().grounded());
		m_player->get_collider().collision_depths = {};
	}
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	post_death.update();
	if (post_death.is_complete()) {
		if (m_player->has_collider()) { m_player->get_collider().collision_depths = util::CollisionDepth(); }
		if (change_state(AnimState::idle, get_params("idle"), true)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::run, get_params("run"), true)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, get_params("sprint"), true)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::rise, get_params("rise"), true)) { return PA_BIND(update_rise); }
		triggers.set(AnimTriggers::end_death);
	}
	return PA_BIND(update_die);
}

fsm::StateFunction PlayerAnimation::update_drown() {
	m_player->animation.label = "drown";
	p_state.actual = AnimState::drown;
	if (m_player->animation.just_started()) {
		m_player->m_services->music_player.stop();
		post_death.start();
		triggers.reset(AnimTriggers::end_death);
	}
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	post_death.update();
	if (post_death.is_complete()) {
		if (m_player->has_collider()) { m_player->get_collider().collision_depths = util::CollisionDepth(); }
		if (change_state(AnimState::idle, get_params("idle"), true)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::run, get_params("run"), true)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, get_params("sprint"), true)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::rise, get_params("rise"), true)) { return PA_BIND(update_rise); }
		triggers.set(AnimTriggers::end_death);
	}
	return PA_BIND(update_drown);
}

fsm::StateFunction PlayerAnimation::update_backflip() {
	m_player->animation.label = "backflip";
	p_state.actual = AnimState::backflip;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::wallslide, get_params("wallslide"), true)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::run, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::sharp_turn, get_params("sharp_turn"))) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, get_params("turn"))) { return PA_BIND(update_turn); }
	if (m_player->animation.complete()) {
		m_player->animation.set_params(get_params("suspend"));
		return PA_BIND(update_suspend);
	}
	return PA_BIND(update_backflip);
}

fsm::StateFunction PlayerAnimation::update_slide() {
	m_player->animation.label = "slide";
	p_state.actual = AnimState::slide;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::get_up, get_params("get_up"))) { return PA_BIND(update_get_up); }
	if (change_state(AnimState::turn_slide, get_params("turn_slide"))) { return PA_BIND(update_turn_slide); }
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
	m_player->animation.label = "get_up";
	p_state.actual = AnimState::get_up;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }

	if (m_player->animation.complete()) {
		m_player->animation.set_params(get_params("idle"));
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
	m_player->animation.label = "roll";
	p_state.actual = AnimState::roll;
	auto& controller = m_player->controller;
	controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::inspect, get_params("inspect"))) {
		m_player->get_collider().physics.stop_x();
		return PA_BIND(update_inspect);
	}
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }

	if (m_player->animation.complete()) {
		if (controller.is_sliding() && !m_player->firing_weapon()) {
			m_player->animation.set_params(get_params("slide"));
			return PA_BIND(update_slide);
		} else {
			if (controller.sprinting()) {
				m_player->animation.set_params(get_params("sprint"));
				return PA_BIND(update_sprint);
			}
			m_player->animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
	}
	return PA_BIND(update_roll);
}

fsm::StateFunction PlayerAnimation::update_turn_slide() {
	m_player->animation.label = "turn_slide";
	p_state.actual = AnimState::turn_slide;
	auto& controller = m_player->controller;
	controller.reset_vertical_movement();
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::inspect, get_params("inspect"))) {
		m_player->get_collider().physics.stop_x();
		return PA_BIND(update_inspect);
	}
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("backflip"))) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }

	if (m_player->animation.complete()) {
		if (controller.is_sliding() && !m_player->firing_weapon()) {
			m_player->animation.set_params(get_params("slide"));
			return PA_BIND(update_slide);
		} else {
			if (controller.sprinting()) {
				m_player->animation.set_params(get_params("sprint"));
				return PA_BIND(update_sprint);
			}
			m_player->animation.set_params(get_params("idle"));
			return PA_BIND(update_idle);
		}
	}
	return PA_BIND(update_turn_slide);
}

fsm::StateFunction PlayerAnimation::update_shoot() {
	m_player->animation.label = "shoot";
	p_state.actual = AnimState::shoot;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
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
	if (m_player->animation.complete()) {
		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_shoot);
}

fsm::StateFunction player::PlayerAnimation::update_sleep() {
	m_player->animation.label = "sleep";
	p_state.actual = AnimState::sleep;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	if (m_player->has_collider()) { m_player->get_collider().physics.zero_x(); }
	if (m_sleep_timer.is_almost_complete()) { request(AnimState::wake_up); }
	if (change_state(AnimState::wake_up, get_params("wake_up"), true)) {
		NANI_LOG_DEBUG(m_logger, "Woke up!");
		return PA_BIND(update_wake_up);
	}
	return PA_BIND(update_sleep);
}

fsm::StateFunction player::PlayerAnimation::update_wake_up() {
	m_player->animation.label = "wake_up";
	p_state.actual = AnimState::wake_up;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	if (m_player->animation.complete()) {
		m_player->set_flag(PlayerFlags::show_weapon);
		m_player->controller.unrestrict();
		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_wake_up);
}

fsm::StateFunction player::PlayerAnimation::update_crouch() {
	m_player->animation.label = "crouch";
	p_state.actual = AnimState::crouch;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (m_player->animation.complete()) {
		if (change_state(AnimState::crawl, get_params("crawl"), true)) { return PA_BIND(update_crawl); }
	}
	if (!m_player->controller.is_crouching()) {
		request(AnimState::get_up);
		if (change_state(AnimState::get_up, get_params("get_up"), true)) { return PA_BIND(update_get_up); }
	}
	return PA_BIND(update_crouch);
}

fsm::StateFunction player::PlayerAnimation::update_crawl() {
	m_player->animation.label = "crawl";
	p_state.actual = AnimState::crawl;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::hurt, get_params("hurt"))) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::suspend, get_params("suspend"))) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, get_params("between_push"))) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::crouch, get_params("crouch"), true)) {
		m_player->animation.set_frame(4);
		return PA_BIND(update_crouch);
	}
	if (!m_player->controller.is_crouching()) {
		request(AnimState::get_up);
		if (change_state(AnimState::get_up, get_params("get_up"), true)) { return PA_BIND(update_get_up); }
	}
	return PA_BIND(update_crawl);
}

fsm::StateFunction player::PlayerAnimation::update_dash_kick() {
	m_player->animation.label = "dash_kick";
	p_state.actual = AnimState::dash_kick;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (m_player->animation.complete()) {
		if (change_state(AnimState::land, get_params("land"), true)) { return PA_BIND(update_land); }
		if (change_state(AnimState::suspend, get_params("suspend"), true)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::wallslide, get_params("wallslide"))) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::shoot, get_params("shoot"))) { return PA_BIND(update_shoot); }
		if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
		if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
		if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
		if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
		m_player->animation.set_params(get_params("idle"));
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_dash_kick);
}

fsm::StateFunction player::PlayerAnimation::update_slow_walk() {
	m_player->animation.label = "slow_walk";
	p_state.actual = AnimState::slow_walk;
	if (m_player->has_collider()) { m_player->get_collider().physics.forced_acceleration.x = m_player->get_actual_direction().as_float() * 0.15f; }
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	return PA_BIND(update_slow_walk);
}

fsm::StateFunction player::PlayerAnimation::update_swim() {
	m_player->animation.label = "swim";
	p_state.actual = AnimState::swim;
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::dive, get_params("dive"))) { return PA_BIND(update_dive); }
	if (change_state(AnimState::suspend, get_params("suspend"), true)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (change_state(AnimState::roll, get_params("roll"), true)) { return PA_BIND(update_roll); }
	if (change_state(AnimState::slide, get_params("slide"))) { return PA_BIND(update_slide); }
	if (change_state(AnimState::rise, get_params("rise"))) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, get_params("sprint"))) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::sprint, get_params("run"))) { return PA_BIND(update_run); }
	if (change_state(AnimState::idle, get_params("idle"), true)) { return PA_BIND(update_idle); }
	return PA_BIND(update_swim);
}

fsm::StateFunction player::PlayerAnimation::update_dive() {
	m_player->animation.label = "dive";
	p_state.actual = AnimState::dive;
	m_player->set_flag(PlayerFlags::show_weapon, false);
	if (change_state(AnimState::die, get_params("die"), true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::drown, get_params("drown"), true)) { return PA_BIND(update_drown); }
	if (change_state(AnimState::dash, get_params("dash"))) { return PA_BIND(update_dash); }
	if (change_state(AnimState::dash_up, get_params("dash_up"))) { return PA_BIND(update_dash_up); }
	if (change_state(AnimState::dash_down, get_params("dash_down"))) { return PA_BIND(update_dash_down); }
	if (m_player->animation.complete()) {
		m_player->set_flag(PlayerFlags::show_weapon);
		if (change_state(AnimState::suspend, get_params("suspend"), true)) { return PA_BIND(update_suspend); }
		if (m_player->has_collider()) {
			m_player->get_collider().has_flag_set(shape::ColliderFlags::in_water) ? request(AnimState::swim) : request(AnimState::suspend);
		} else {
			request(AnimState::idle);
		}
		if (change_state(AnimState::swim, get_params("swim"), true)) { return PA_BIND(update_swim); }
		if (change_state(AnimState::idle, get_params("idle"), true)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::suspend, get_params("suspend"), true)) { return PA_BIND(update_suspend); }
	}
	return PA_BIND(update_dive);
}

bool PlayerAnimation::change_state(AnimState next, anim::Parameters params, bool hard) {
	if (was_requested(next)) {
		m_player->animation.set_params(params, true);
		return true;
	}
	return false;
}

void player::PlayerAnimation::force(AnimState to_state, std::string_view key) {
	request(to_state);
	change_state(to_state, get_params(key.data()), true);
	m_buffer.start();
}

anim::Parameters const& player::PlayerAnimation::get_params(std::string const& key) { return m_player->get_params(key); }

} // namespace fornani::player
