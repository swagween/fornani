
#include "fornani/entities/enemy/catalog/Hulmet.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Hulmet::Hulmet(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, map, "hulmet"), m_services(&svc), m_map(&map), m_parts{.gun{svc, "hulmet_gun", {24, 10}, {{0, 4, 24, -1}}, {"main"}, 2.0f, 0.85f, {-12.f, 6.f}}}, m_weapon{svc, "skycorps_ar"}, m_jump_force{-48.f} {
	animation.set_params(m_animations.idle);
	m_parts.gun.set_magnitude(2.f);
	m_weapon.clip_cooldown_time = 360;
	m_weapon.get().set_team(arms::Team::skycorps);
	get_secondary_collider().set_dimensions({24.f, 24.f});
	flags.general.set(GeneralFlags::invincible_secondary);
	m_cooldowns.run.start();
}

void Hulmet::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }
	if (svc.ticker.every_second()) {
		if (random::percent_chance(20)) { request(HulmetState::run); }
	}
	face_player(player);
	if (is_alert() && !m_cooldowns.post_fire.running() && was_alerted()) { request(HulmetState::shoot); }
	if (is_hostile() && !m_cooldowns.post_roll.running()) { request(HulmetState::roll); }
	if (alertness_triggered() && !was_alerted()) { random::percent_chance(50) ? request(HulmetState::panic) : request(HulmetState::alert); }
	if (directions.actual.lnr != directions.desired.lnr) { request(HulmetState::turn); }
	auto detected_projectile = m_caution.projectile_detected(map, physical.hostile_range, arms::Team::skycorps);
	auto towards_me = (detected_projectile.left() && directions.actual.right()) || (detected_projectile.right() && directions.actual.left());
	if (towards_me && get_collider().grounded() && !m_cooldowns.post_roll.running()) { request(HulmetState::roll); }
	if (detected_projectile.up_or_down() && !m_cooldowns.post_roll.running()) { request(HulmetState::roll); }
	if (is_hurt()) { m_cooldowns.post_roll.running() ? request(HulmetState::panic) : request(HulmetState::roll); }
	if (m_caution.detected_step(map, get_collider(), directions.actual) && (get_collider().physics.is_moving_horizontally(0.5f) || is_mid_run()) && !m_cooldowns.post_jump.running()) { request(HulmetState::jump); }
	if (is_out_of_ammo()) { request(HulmetState::reload); }

	m_cooldowns.post_fire.update();
	m_cooldowns.post_jump.update();
	m_cooldowns.post_roll.update();
	m_cooldowns.alerted.update();

	m_parts.gun.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
	m_weapon.update(svc, map, *this);
	m_weapon.barrel_offset = sf::Vector2f{directions.actual.as_float() * 40.f, 0.f};
	if (p_state.actual == HulmetState::roll) { cancel_shake(); }

	if (secondary_collider) { get_secondary_collider().set_position(get_collider().bounding_box.get_position() + sf::Vector2f{4.f, -8.f}); }

	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.flags.hulmet.set(audio::Hulmet::hurt); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	state_function = state_function();
}

void Hulmet::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (p_state.actual != HulmetState::roll && p_state.actual != HulmetState::sleep) {
		if (m_parts.gun.animated_sprite) {
			m_parts.gun.animated_sprite->set_channel(has_flag_set(HulmetFlags::out_of_ammo) ? 0 : 1);
			m_parts.gun.render(svc, win, cam);
		}
	}

	if (svc.greyblock_mode()) {}
}

fsm::StateFunction Hulmet::update_idle() {
	animation.label = "idle";
	p_state.actual = HulmetState::idle;
	flags.state.set(StateFlags::vulnerable);
	if (is_hurt()) { request(HulmetState::panic); }
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::panic, m_animations.panic)) { return HULMET_BIND(update_panic); }
	if (change_state(HulmetState::reload, m_animations.reload)) { return HULMET_BIND(update_reload); }
	if (change_state(HulmetState::sleep, m_animations.sleep)) { return HULMET_BIND(update_sleep); }
	if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
	if (change_state(HulmetState::jump, m_animations.jump)) {
		impulse.start(m_jump_time);
		return HULMET_BIND(update_jump);
	}
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (was_alerted()) {
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	}
	return HULMET_BIND(update_idle);
}

fsm::StateFunction Hulmet::update_turn() {
	animation.label = "turn";
	p_state.actual = HulmetState::turn;
	flags.state.set(StateFlags::vulnerable);
	directions.desired.lock();
	if (animation.complete()) {
		request_flip();
		if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (was_alerted()) {
			if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		}
		animation.set_params(m_animations.idle);
		p_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_turn);
}

fsm::StateFunction Hulmet::update_run() {
	animation.label = "run";
	p_state.actual = HulmetState::run;
	flags.state.set(StateFlags::vulnerable);
	auto sign = directions.actual.left() ? -1.f : 1.f;
	get_collider().physics.apply_force({sign * attributes.speed, 0.f});
	m_cooldowns.run.update();
	if (m_caution.detected_step(*m_map, get_collider(), directions.actual)) {
		request(HulmetState::jump);
		m_cooldowns.run.start();
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
	}
	if (change_state(HulmetState::turn, m_animations.turn)) {
		m_cooldowns.run.start();
		return HULMET_BIND(update_turn);
	}
	if (change_state(HulmetState::idle, m_animations.idle)) {
		m_cooldowns.run.start();
		return HULMET_BIND(update_idle);
	}
	if (change_state(HulmetState::alert, m_animations.alert)) {
		m_cooldowns.run.start();
		return HULMET_BIND(update_alert);
	}
	if (animation.complete()) {
		if (change_state(HulmetState::roll, m_animations.roll)) {
			m_cooldowns.run.start();
			return HULMET_BIND(update_roll);
		}
	}
	if (change_state(HulmetState::jump, m_animations.jump)) {
		impulse.start(m_jump_time);
		m_cooldowns.run.start();
		return HULMET_BIND(update_jump);
	}
	if (animation.complete()) {
		animation.set_params(m_animations.idle);
		p_state.desired = HulmetState::idle;
		m_cooldowns.run.start();
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_run);
}

fsm::StateFunction Hulmet::update_alert() {
	animation.label = "alert";
	p_state.actual = HulmetState::alert;
	flags.state.set(StateFlags::vulnerable);
	m_cooldowns.alerted.start();
	if (animation.just_started()) { m_services->soundboard.flags.hulmet.set(audio::Hulmet::alert); }
	if (animation.complete()) {
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
		if (!is_out_of_ammo()) { request(HulmetState::shoot); }
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	}
	return HULMET_BIND(update_alert);
}

fsm::StateFunction Hulmet::update_roll() {
	animation.label = "roll";
	p_state.actual = HulmetState::roll;
	flags.state.reset(StateFlags::vulnerable);
	auto sign = directions.actual.left() ? -1.f : 1.f;
	get_collider().physics.apply_force({sign * attributes.speed * 2.f, 0.f});
	if (m_caution.detected_step(*m_map, get_collider(), directions.movement)) {
		request(HulmetState::jump);
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
	}
	if (animation.complete()) {
		m_cooldowns.post_roll.start();
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (!is_out_of_ammo()) {
			if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		}
		animation.set_params(m_animations.idle);
		p_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_roll);
}

fsm::StateFunction Hulmet::update_jump() {
	animation.label = "jump";
	p_state.actual = HulmetState::jump;
	flags.state.set(StateFlags::vulnerable);
	if (impulse.running()) { get_collider().physics.apply_force({0, m_jump_force}); }
	auto sign = directions.actual.left() ? -1.f : 1.f;
	get_collider().physics.apply_force({sign * attributes.speed, 0.f});
	if (animation.complete()) {
		m_cooldowns.post_jump.start();
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		animation.set_params(m_animations.idle);
		p_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_jump);
}

fsm::StateFunction Hulmet::update_shoot() {
	animation.label = "shoot";
	p_state.actual = HulmetState::shoot;
	flags.state.set(StateFlags::vulnerable);
	if (!m_weapon.get().cooling_down()) { m_weapon.shoot(*m_services, *m_map); }
	if (animation.complete()) {
		set_flag(HulmetFlags::out_of_ammo);
		m_cooldowns.post_fire.start();
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
		animation.set_params(m_animations.idle);
		p_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_shoot);
}

fsm::StateFunction Hulmet::update_sleep() {
	animation.label = "sleep";
	p_state.actual = HulmetState::sleep;
	flags.state.set(StateFlags::vulnerable);
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::shoot, m_animations.alert)) { return HULMET_BIND(update_alert); }
	return HULMET_BIND(update_sleep);
}

fsm::StateFunction Hulmet::update_panic() {
	animation.label = "panic";
	p_state.actual = HulmetState::panic;
	flags.state.set(StateFlags::vulnerable);
	m_cooldowns.alerted.start();
	if (animation.just_started()) { m_services->soundboard.flags.hulmet.set(audio::Hulmet::alert); }
	if (animation.complete()) {
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		random::percent_chance(50) && !is_out_of_ammo() ? request(HulmetState::shoot) : request(HulmetState::roll);
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	}
	return HULMET_BIND(update_panic);
}

fsm::StateFunction Hulmet::update_reload() {
	animation.label = "reload";
	p_state.actual = HulmetState::reload;
	flags.state.set(StateFlags::vulnerable);
	if (animation.get_frame_count() == 3 && animation.keyframe_started()) {
		m_services->soundboard.flags.hulmet.set(audio::Hulmet::reload);
		set_flag(HulmetFlags::out_of_ammo, false);
		if (m_parts.gun.animated_sprite) { m_parts.gun.animated_sprite->animation.start(); }
	}
	if (animation.complete()) {
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		is_hostile() ? request(HulmetState::shoot) : request(HulmetState::idle);
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		if (change_state(HulmetState::idle, m_animations.idle)) { return HULMET_BIND(update_idle); }
	}
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	return HULMET_BIND(update_reload);
}

bool Hulmet::change_state(HulmetState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
