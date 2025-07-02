
#include "fornani/entities/enemy/catalog/Hulmet.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Hulmet::Hulmet(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "hulmet"), m_services(&svc), m_map(&map), m_parts{.gun{svc.assets.get_texture("hulmet_gun"), 2.0f, 0.85f, {-12.f, 6.f}}}, m_weapon{svc, 0} {
	animation.set_params(m_animations.idle);
	m_parts.gun.set_magnitude(2.f);
	m_weapon.clip_cooldown_time = 360;
	m_weapon.get().set_team(arms::Team::skycorps);
	secondary_collider = shape::Collider({32.f, 16.f});
	flags.general.set(GeneralFlags::invincible_secondary);
	m_cooldowns.run.start();
}

void Hulmet::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }
	if (svc.ticker.every_second()) {
		if (util::random::percent_chance(20)) { request(HulmetState::run); }
	}
	face_player(player);
	if (alert() && !m_cooldowns.post_fire.running()) { request(HulmetState::shoot); }
	if (hostile()) { request(HulmetState::roll); }
	if (alertness_triggered() && !m_cooldowns.alerted.running()) { request(HulmetState::alert); }
	if (directions.actual.lnr != directions.desired.lnr) { request(HulmetState::turn); }
	auto detected_projectile = m_caution.projectile_detected(map, physical.hostile_range, arms::Team::skycorps);
	auto towards_me = (detected_projectile.left() && directions.actual.right()) || (detected_projectile.right() && directions.actual.left());
	if (towards_me && collider.grounded()) { request(HulmetState::roll); }
	if (detected_projectile.up_or_down()) { request(HulmetState::roll); }
	if (m_caution.detected_step(map, collider, directions.actual) && (collider.physics.is_moving_horizontally(0.5f) || is_mid_run()) && !m_cooldowns.post_jump.running()) { request(HulmetState::jump); }

	m_cooldowns.post_fire.update();
	m_cooldowns.post_jump.update();
	m_cooldowns.alerted.update();

	m_parts.gun.update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center());
	m_weapon.update(svc, map, *this);
	auto sign = directions.actual.left() ? -1.f : 1.f;
	m_weapon.barrel_offset = sf::Vector2f{12.f * sign, 6.f};

	secondary_collider.set_position(collider.bounding_box.get_position() + sf::Vector2f{0.f, -16.f});

	state_function = state_function();
}

void Hulmet::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (m_state.actual != HulmetState::roll && m_state.actual != HulmetState::sleep) { m_parts.gun.render(svc, win, cam); }

	if (svc.greyblock_mode()) {}
}

fsm::StateFunction Hulmet::update_idle() {
	animation.label = "idle";
	m_state.actual = HulmetState::idle;
	flags.state.set(StateFlags::vulnerable);
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::sleep, m_animations.sleep)) { return HULMET_BIND(update_sleep); }
	if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
	if (change_state(HulmetState::jump, m_animations.jump)) {
		impulse.start(m_jump_time);
		return HULMET_BIND(update_jump);
	}
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	return HULMET_BIND(update_idle);
}

fsm::StateFunction Hulmet::update_turn() {
	animation.label = "turn";
	m_state.actual = HulmetState::turn;
	flags.state.set(StateFlags::vulnerable);
	if (animation.complete()) {
		request_flip();
		if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		animation.set_params(m_animations.idle);
		m_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_turn);
}

fsm::StateFunction Hulmet::update_run() {
	animation.label = "run";
	m_state.actual = HulmetState::run;
	flags.state.set(StateFlags::vulnerable);
	auto sign = directions.actual.left() ? -1.f : 1.f;
	collider.physics.apply_force({sign * 0.4f, 0.f});
	m_cooldowns.run.update();
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
	if (change_state(HulmetState::roll, m_animations.roll)) {
		m_cooldowns.run.start();
		return HULMET_BIND(update_roll);
	}
	if (change_state(HulmetState::jump, m_animations.jump)) {
		impulse.start(m_jump_time);
		m_cooldowns.run.start();
		return HULMET_BIND(update_jump);
	}
	if (animation.complete()) {
		animation.set_params(m_animations.idle);
		m_state.desired = HulmetState::idle;
		m_cooldowns.run.start();
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_run);
}

fsm::StateFunction Hulmet::update_alert() {
	animation.label = "alert";
	m_state.actual = HulmetState::alert;
	flags.state.set(StateFlags::vulnerable);
	m_cooldowns.alerted.start();
	if (animation.complete()) {
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
		animation.set_params(m_animations.shoot);
		m_state.desired = HulmetState::shoot;
		return HULMET_BIND(update_shoot);
	}
	return HULMET_BIND(update_alert);
}

fsm::StateFunction Hulmet::update_roll() {
	animation.label = "roll";
	m_state.actual = HulmetState::roll;
	flags.state.reset(StateFlags::vulnerable);
	auto sign = directions.actual.left() ? -1.f : 1.f;
	collider.physics.apply_force({sign * 0.8f, 0.f});
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::jump, m_animations.jump)) {
		impulse.start(m_jump_time);
		return HULMET_BIND(update_jump);
	}
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	if (animation.complete()) {
		animation.set_params(m_animations.idle);
		m_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_roll);
}

fsm::StateFunction Hulmet::update_jump() {
	animation.label = "jump";
	m_state.actual = HulmetState::jump;
	flags.state.set(StateFlags::vulnerable);
	if (impulse.running()) { collider.physics.apply_force({0, m_jump_force}); }
	auto sign = directions.actual.left() ? -1.f : 1.f;
	collider.physics.apply_force({sign * 0.4f, 0.f});
	if (animation.complete()) {
		m_cooldowns.post_jump.start();
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
		animation.set_params(m_animations.idle);
		m_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_jump);
}

fsm::StateFunction Hulmet::update_shoot() {
	animation.label = "shoot";
	m_state.actual = HulmetState::shoot;
	flags.state.set(StateFlags::vulnerable);
	if (!m_weapon.get().cooling_down()) {
		m_weapon.shoot();
		m_map->spawn_projectile_at(*m_services, m_weapon.get(), m_weapon.barrel_point());
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (animation.complete()) {
		m_cooldowns.post_fire.start();
		if (is_hurt()) { request(HulmetState::roll); }
		if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
		if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
		if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
		if (change_state(HulmetState::jump, m_animations.jump)) {
			impulse.start(m_jump_time);
			return HULMET_BIND(update_jump);
		}
		animation.set_params(m_animations.idle);
		m_state.desired = HulmetState::idle;
		return HULMET_BIND(update_idle);
	}
	return HULMET_BIND(update_shoot);
}

fsm::StateFunction Hulmet::update_sleep() {
	animation.label = "sleep";
	m_state.actual = HulmetState::sleep;
	flags.state.set(StateFlags::vulnerable);
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::shoot, m_animations.alert)) { return HULMET_BIND(update_alert); }
	return HULMET_BIND(update_sleep);
}

bool Hulmet::change_state(HulmetState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
