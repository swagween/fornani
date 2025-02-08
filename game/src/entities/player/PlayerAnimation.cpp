#include "fornani/entities/player/PlayerAnimation.hpp"

#include <iostream>

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

PlayerAnimation::PlayerAnimation(Player& plr) : m_player(&plr) {
	state_function = state_function();
	animation.set_params(idle);
	animation.start();
	state = AnimState::idle;
}

void PlayerAnimation::update() {
	cooldowns.walljump.update();
	animation.update();
	state_function = state_function();
	if (m_player->is_dead()) { state = AnimState::die; }
}

void PlayerAnimation::start() { animation.start(); }

bool PlayerAnimation::stepped() const { return (animation.get_frame() == 44 || animation.get_frame() == 46 || animation.get_frame() == 10 || animation.get_frame() == 13 || animation.get_frame() == 16) && animation.keyframe_started(); }

fsm::StateFunction PlayerAnimation::update_idle() {
	animation.label = "idle";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (animation.just_started()) {
		idle_timer.start();
		m_player->cooldowns.push.start();
	}
	idle_timer.update();
	if (idle_timer.get_count() > timers.sit) { state = AnimState::sit; }
	if (change_state(AnimState::sit, sit)) {
		idle_timer.cancel();
		return PA_BIND(update_sit);
	}
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::shoot, shoot)) { return PA_BIND(update_shoot); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }

	state = AnimState::idle;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_sprint() {
	animation.label = "sprint";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::slide, slide, true)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }

	state = AnimState::sprint;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_shield() {
	animation.label = "shield";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }

	state = AnimState::shield;
	return PA_BIND(update_shield);
}

fsm::StateFunction PlayerAnimation::update_between_push() {
	animation.label = "between push";
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (animation.complete()) {
		if (change_state(AnimState::push, push)) { return PA_BIND(update_push); }
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}

	state = AnimState::between_push;
	return PA_BIND(update_between_push);
}

fsm::StateFunction PlayerAnimation::update_push() {
	animation.label = "push";
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (state != AnimState::push) {
		state = AnimState::between_push;
		animation.set_params(between_push);
		return PA_BIND(update_between_push);
	}

	return PA_BIND(update_push);
}

fsm::StateFunction PlayerAnimation::update_run() {
	animation.label = "run";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }

	state = AnimState::run;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_turn() {
	animation.label = "turn";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (animation.complete()) {
		triggers.set(AnimTriggers::flip);
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}

	state = AnimState::turn;
	return PA_BIND(update_turn);
}

fsm::StateFunction PlayerAnimation::update_sharp_turn() {
	animation.label = "sharp_turn";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (animation.complete()) {
		triggers.set(AnimTriggers::flip);
		if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}

	state = AnimState::sharp_turn;
	return PA_BIND(update_sharp_turn);
}

fsm::StateFunction PlayerAnimation::update_rise() {
	animation.label = "rise";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		if (m_player->grounded()) {
			state = AnimState::idle;
			animation.set_params(idle);
			return PA_BIND(update_idle);
		}
		state = AnimState::suspend;
		animation.set_params(suspend);
		return PA_BIND(update_suspend);
	}

	state = AnimState::rise;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_suspend() {
	animation.label = "suspend";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }

	state = AnimState::suspend;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_fall() {
	animation.label = "fall";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::roll, roll)) { return PA_BIND(update_roll); }
	if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if(m_player->grounded()) {
		state = AnimState::land;
		animation.set_params(land);
		return PA_BIND(update_land);
	}

	state = AnimState::fall;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_stop() {
	animation.label = "stop";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }

	if (animation.complete()) {
		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }

	state = AnimState::stop;
	return PA_BIND(update_stop);
}

fsm::StateFunction PlayerAnimation::update_inspect() {
	animation.label = "inspect";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	}
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }

	state = AnimState::inspect;
	return PA_BIND(update_inspect);
}

fsm::StateFunction PlayerAnimation::update_sit() {
	animation.label = "sit";
	m_player->flags.state.reset(State::show_weapon);
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (animation.complete()) {
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	}
	if (m_player->arsenal && m_player->hotbar) {
		if (m_player->equipped_weapon().cooling_down()) {
			state = AnimState::idle;
			animation.set_params(idle);
			return PA_BIND(update_idle);
		}
	}
	state = AnimState::sit;
	return PA_BIND(update_sit);
}

fsm::StateFunction PlayerAnimation::update_land() {
	animation.label = "land";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (animation.complete()) {
		m_player->controller.get_slide().end();
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	state = AnimState::land;
	return PA_BIND(update_land);
}

fsm::StateFunction PlayerAnimation::update_hurt() {
	animation.label = "hurt";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (animation.complete()) {
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
		if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
		if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}

	state = AnimState::hurt;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_dash() {
	animation.label = "dash";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (animation.complete()) {
		if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
		if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
		if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }

		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}

	state = AnimState::dash;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_wallslide() {
	animation.label = "wallslide";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::walljump, walljump, true)) {
		cooldowns.walljump.start();
		return PA_BIND(update_walljump);
	}
	if (change_state(AnimState::rise, walljump, true)) {
		cooldowns.walljump.start();
		return PA_BIND(update_walljump);
	}
	if (change_state(AnimState::backflip, backflip)) { return PA_BIND(update_backflip); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::land, land)) { return PA_BIND(update_land); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }

	state = AnimState::wallslide;
	return PA_BIND(update_wallslide);
}

fsm::StateFunction PlayerAnimation::update_walljump() {
	animation.label = "walljump";
	if (cooldowns.walljump.running()) {
		auto sign = m_player->moving_left() ? -1.f : 1.f;
		if (abs(m_player->collider.physics.apparent_velocity().x) < 0.01f) { sign = m_player->controller.facing_left() ? 1.f : -1.f; }
		m_player->collider.physics.forced_acceleration = {0.8f * sign, 0.f};
		m_player->controller.stop();
	} else {
		m_player->controller.stop_walljumping();
	}
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		state = AnimState::suspend;
		animation.set_params(suspend);
		return PA_BIND(update_suspend);
	}
	state = AnimState::walljump;
	return PA_BIND(update_walljump);
}

fsm::StateFunction PlayerAnimation::update_die() {
	animation.label = "die";
	if (animation.just_started()) {
		m_player->m_services->music.stop();
		post_death.start();
		triggers.reset(AnimTriggers::end_death);
		m_player->m_services->state_controller.actions.set(automa::Actions::death_mode); //set here, reset on map load
		//std::cout << "Death animation started.\n";
	}
	m_player->controller.restrict_movement();
	m_player->controller.prevent_movement();
	m_player->collider.collision_depths = {};
	post_death.update();
	if (!m_player->m_services->death_mode()) {
		m_player->collider.collision_depths = util::CollisionDepth();
		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	if (post_death.is_complete()) {
		m_player->collider.collision_depths = util::CollisionDepth();
		if (change_state(AnimState::idle, idle, true)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::run, run, true)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, sprint, true)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
		if (change_state(AnimState::rise, rise, true)) { return PA_BIND(update_rise); }
		triggers.set(AnimTriggers::end_death);
	}
	state = AnimState::die;
	return PA_BIND(update_die);
}

fsm::StateFunction PlayerAnimation::update_backflip() {
	animation.label = "backflip";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (animation.complete()) {
		state = AnimState::suspend;
		animation.set_params(suspend);
		return PA_BIND(update_suspend);
	}

	state = AnimState::backflip;
	return PA_BIND(update_backflip);
}

fsm::StateFunction PlayerAnimation::update_slide() {
	animation.label = "slide";
	auto& slider = m_player->controller.get_slide();
	slider.calculate();
	if (!slider.started()) {
		slider.start();
		slider.direction = m_player->controller.direction;
	}
	if (animation.just_started()) {
		m_player->m_services->soundboard.flags.player.set(audio::Player::slide);
		slider.slide();
	}
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) {
		m_player->controller.get_slide().end();
		return PA_BIND(update_rise);
	}

	if (slider.broke_out() && !m_player->controller.roll.rolling()) {
		m_player->controller.get_slide().end();
		state = AnimState::get_up;
		animation.set_params(get_up);
		return PA_BIND(update_get_up);
	}
	m_player->controller.roll.break_out();

	if (!m_player->grounded()) {
		m_player->controller.get_slide().end();
		state = AnimState::suspend;
		animation.set_params(suspend);
		return PA_BIND(update_suspend);
	}

	// physics
	if (m_player->collider.downhill()) { slider.start(); }
	m_player->collider.physics.acceleration.x = slider.get_speed() * m_player->controller.sliding_movement() * slider.get_dampen();

	if (slider.direction.lr != m_player->controller.direction.lr) {
		std::cout << "slider dir\n";
		m_player->controller.get_slide().end();
		state = AnimState::sharp_turn;
		animation.set_params(sharp_turn);
		return PA_BIND(update_sharp_turn);
	}
	if (m_player->controller.get_slide().can_exit()) {
		if (state != AnimState::slide) { m_player->controller.get_slide().end(); }
		if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
		if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
		if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
		if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
		if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
		if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
		if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
		if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
		if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
		if (change_state(AnimState::idle, idle)) { return PA_BIND(update_idle); }
		if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
		if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
		if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
		if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
		if (!m_player->controller.moving() || slider.exhausted() || !m_player->controller.sliding()) {
			m_player->controller.get_slide().end();
			state = AnimState::get_up;
			animation.set_params(get_up);
			std::cout << "exited\n";
			return PA_BIND(update_get_up);
		}
	}

	state = AnimState::slide;
	return std::move(state_function);
}

fsm::StateFunction PlayerAnimation::update_get_up() {
	animation.label = "get_up";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }

	if (animation.complete()) {
		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::run, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }

	state = AnimState::get_up;
	return PA_BIND(update_get_up);
}

fsm::StateFunction PlayerAnimation::update_roll() {
	animation.label = "roll";
	auto& controller = m_player->controller;
	auto sign = m_player->controller.facing_left() ? -1.f: 1.f;
	m_player->collider.physics.velocity.x = 60.f * sign;
	if (!controller.roll.rolling()) {
		controller.roll.direction.lr = controller.direction.lr;
		m_player->m_services->soundboard.flags.player.set(audio::Player::roll);
	}
	controller.roll.roll();
	if (controller.roll.direction.lr != controller.direction.lr) {
		m_player->collider.physics.hard_stop_x();
		state = AnimState::sharp_turn;
		animation.set_params(sharp_turn);
		return PA_BIND(update_sharp_turn);
	}
	if (change_state(AnimState::inspect, inspect)) {
		m_player->collider.physics.stop_x();
		controller.roll.break_out();
		return PA_BIND(update_inspect);
	}
	if (change_state(AnimState::die, die, true)) {
		controller.roll.break_out();
		return PA_BIND(update_die);
	}
	if (change_state(AnimState::hurt, hurt)) {
		controller.roll.break_out();
		return PA_BIND(update_hurt);
	}
	if (change_state(AnimState::rise, rise)) {
		controller.roll.break_out();
		return PA_BIND(update_rise);
	}
	if (change_state(AnimState::suspend, suspend)) {
		controller.roll.break_out();
		return PA_BIND(update_suspend);
	}

	if (animation.complete()) {
		if (controller.sliding()) {
			state = AnimState::slide;
			animation.set_params(slide);
			return PA_BIND(update_slide);
		} else {
			controller.roll.break_out();
			state = AnimState::idle;
			animation.set_params(idle);
			return PA_BIND(update_idle);
		}
	}
	state = AnimState::roll;
	return PA_BIND(update_roll);
}

fsm::StateFunction PlayerAnimation::update_shoot() {
	animation.label = "shoot";
	if (change_state(AnimState::die, die, true)) { return PA_BIND(update_die); }
	if (change_state(AnimState::rise, rise)) { return PA_BIND(update_rise); }
	if (change_state(AnimState::sprint, sprint)) { return PA_BIND(update_sprint); }
	if (change_state(AnimState::sprint, run)) { return PA_BIND(update_run); }
	if (change_state(AnimState::shoot, shoot)) { return PA_BIND(update_shoot); }
	if (change_state(AnimState::slide, slide)) { return PA_BIND(update_slide); }
	if (change_state(AnimState::dash, dash)) { return PA_BIND(update_dash); }
	if (change_state(AnimState::push, between_push)) { return PA_BIND(update_between_push); }
	if (change_state(AnimState::inspect, inspect)) { return PA_BIND(update_inspect); }
	if (change_state(AnimState::stop, stop)) { return PA_BIND(update_stop); }
	if (change_state(AnimState::wallslide, wallslide)) { return PA_BIND(update_wallslide); }
	if (change_state(AnimState::suspend, suspend)) { return PA_BIND(update_suspend); }
	if (change_state(AnimState::fall, fall)) { return PA_BIND(update_fall); }
	if (change_state(AnimState::shield, shield)) { return PA_BIND(update_shield); }
	if (change_state(AnimState::hurt, hurt)) { return PA_BIND(update_hurt); }
	if (change_state(AnimState::sharp_turn, sharp_turn)) { return PA_BIND(update_sharp_turn); }
	if (change_state(AnimState::turn, turn)) { return PA_BIND(update_turn); }
	if(animation.complete()) {
		state = AnimState::idle;
		animation.set_params(idle);
		return PA_BIND(update_idle);
	}
	return PA_BIND(update_shoot);
}

bool PlayerAnimation::change_state(AnimState next, anim::Parameters params, bool hard) {
	if (state == next) {
		animation.set_params(params, hard);
		return true;
	}
	return false;
}

} // namespace player
