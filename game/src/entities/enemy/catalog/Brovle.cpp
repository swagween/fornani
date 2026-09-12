
#include <fornani/entities/enemy/catalog/Brovle.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto brovle_framerate = 16;

Brovle::Brovle(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "brovle"), m_services{&svc}, m_jump_time{64}, m_switch_sides{2400} {
	p_animatable.set_animations({{"idle", {0, 4, brovle_framerate * 3, 2}},
								 {"sweep", {4, 5, brovle_framerate * 2, 0}},
								 {"run", {9, 4, brovle_framerate * 2, 1}},
								 {"land", {19, 2, brovle_framerate * 2, 0}},
								 {"jumpsquat", {14, 1, brovle_framerate, 0}},
								 {"jump", {16, 4, brovle_framerate * 2, -1}},
								 {"turn", {27, 5, brovle_framerate * 2, 0}},
								 {"slash", {21, 6, brovle_framerate * 2, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = BrovleState::idle;

	m_variant = static_cast<BrovleVariant>(variant);
	m_sword_wave = entity::WeaponPackage{svc, "javelin"};
	m_sword_wave->get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.95f, 0.99f});
}

void Brovle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { svc.soundboard.play_sound("beast_damage", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_jump_time.update();
	m_switch_sides.update();
	if (m_switch_sides.is_complete()) { m_switch_sides.start(); }

	// bomb variant stuff
	if (m_sword_wave) { m_sword_wave->update(svc, map, *this); }

	// shoot
	if (has_flag_set(BrovleFlags::projectile) && !health.is_dead()) {
		auto bp = sf::Vector2f{directions.actual.as_float() * 10.f, -8.f};
		m_sword_wave->get().set_barrel_point(get_collider().get_center() + bp);
		m_sword_wave->get().shoot(svc, map, player.get_collider().get_center() - get_collider().get_center() + bp);
		set_flag(BrovleFlags::projectile, false);
	}
	if (m_caution.detected_step(map, get_collider(), directions.actual)) { set_flag(BrovleFlags::step_detected); }

	if (directions.actual.lnr != directions.desired.lnr) { request(BrovleState::turn); }
	if (m_caution.is_projectile_detected(map, physical.hostile_range, arms::Team::guardian)) { request(BrovleState::jumpsquat); }

	state_function = state_function();
}

void Brovle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Brovle::update_idle() {
	p_state.actual = BrovleState::idle;
	if (change_state(BrovleState::turn, get_params("turn"))) { return BROVLE_BIND(update_turn); }
	if (get_collider().grounded()) {
		if (change_state(BrovleState::jumpsquat, get_params("jumpsquat"))) { return BROVLE_BIND(update_jumpsquat); }
	}
	if (p_animatable.animation.is_complete()) {
		is_alert() ? request(BrovleState::sweep) : request(BrovleState::run);
		if (change_state(BrovleState::run, get_params("run"))) { return BROVLE_BIND(update_run); }
		if (change_state(BrovleState::sweep, get_params("sweep"))) { return BROVLE_BIND(update_sweep); }
	}
	return BROVLE_BIND(update_idle);
}

fsm::StateFunction Brovle::update_run() {
	p_state.actual = BrovleState::run;
	get_collider().physics.velocity.x = directions.actual.as_float() * 8.f;
	if (change_state(BrovleState::turn, get_params("turn"))) { return BROVLE_BIND(update_turn); }
	if (p_animatable.animation.is_complete()) {
		if (get_collider().grounded()) {
			if (consume_flag(BrovleFlags::step_detected)) { request(BrovleState::jumpsquat); }
			if (change_state(BrovleState::jumpsquat, get_params("jumpsquat"))) { return BROVLE_BIND(update_jumpsquat); }
		}
		is_alert() ? request(BrovleState::sweep) : request(BrovleState::idle);
		if (change_state(BrovleState::sweep, get_params("sweep"))) { return BROVLE_BIND(update_sweep); }
		if (change_state(BrovleState::idle, get_params("idle"))) { return BROVLE_BIND(update_idle); }
	}
	return BROVLE_BIND(update_run);
}

fsm::StateFunction Brovle::update_jumpsquat() {
	p_state.actual = BrovleState::jumpsquat;
	if (p_animatable.animation.complete()) {
		m_jump_time.start();
		m_services->soundboard.play_sound("mid_jump", get_collider().get_center());
		request(BrovleState::jump);
		if (change_state(BrovleState::jump, get_params("jump"))) { return BROVLE_BIND(update_jump); }
	}
	return BROVLE_BIND(update_jumpsquat);
}

fsm::StateFunction Brovle::update_jump() {
	p_state.actual = BrovleState::jump;
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity.y = -24.f; }
	get_collider().physics.velocity.x = directions.actual.as_float() * 4.f;
	if (m_jump_time.is_complete() && get_collider().grounded()) {
		request(BrovleState::land);
		if (change_state(BrovleState::land, get_params("land"))) { return BROVLE_BIND(update_land); }
	}
	return BROVLE_BIND(update_jump);
}

fsm::StateFunction Brovle::update_slash() {
	p_state.actual = BrovleState::slash;
	if (p_animatable.frame_action(4)) {}
	if (p_animatable.animation.is_complete()) {
		request(BrovleState::idle);
		if (change_state(BrovleState::idle, get_params("idle"))) { return BROVLE_BIND(update_idle); }
	}
	return BROVLE_BIND(update_slash);
}

fsm::StateFunction Brovle::update_sweep() {
	p_state.actual = BrovleState::sweep;
	if (p_animatable.frame_action(4)) {}
	if (p_animatable.animation.is_complete()) {
		request(BrovleState::idle);
		if (change_state(BrovleState::idle, get_params("idle"))) { return BROVLE_BIND(update_idle); }
	}
	return BROVLE_BIND(update_sweep);
}

fsm::StateFunction Brovle::update_land() {
	p_state.actual = BrovleState::land;
	if (p_animatable.animation.complete()) {
		is_alert() ? request(BrovleState::slash) : request(BrovleState::run);
		if (change_state(BrovleState::run, get_params("run"))) { return BROVLE_BIND(update_run); }
		if (change_state(BrovleState::slash, get_params("slash"))) { return BROVLE_BIND(update_slash); }
	}
	return BROVLE_BIND(update_land);
}

fsm::StateFunction Brovle::update_turn() {
	p_state.actual = BrovleState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		is_alert() ? request(BrovleState::sweep) : request(BrovleState::run);
		if (change_state(BrovleState::run, get_params("run"))) { return BROVLE_BIND(update_run); }
		if (change_state(BrovleState::sweep, get_params("sweep"))) { return BROVLE_BIND(update_sweep); }
	}
	return BROVLE_BIND(update_turn);
}

bool Brovle::change_state(BrovleState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
