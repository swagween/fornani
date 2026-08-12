
#include <fornani/entities/enemy/catalog/Grappler.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Grappler::Grappler(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "grappler"), m_services(&svc), m_map(&map), m_hold_time{560}, m_leap_cooldown{400} {
	p_animatable.set_animations({{"idle", {0, 4, 40, -1}}, {"leap", {4, 5, 40, 0}}, {"snag", {9, 1, 40, -1}}, {"release", {10, 2, 40, 0}}, {"turn", {12, 3, 40, 0}}, {"whiff", {1, 1, 40, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.99f, 0.99f});
	attributes.team = arms::Team::beast;
	m_grab.set_constant_radius(40.f);
	flags.state.set(StateFlags::vulnerable);
}

void Grappler::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	m_hold_time.update();
	m_leap_cooldown.update();

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.play_sound("hit_low"); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	if (is_state(GrapplerState::snag) && m_flags.test(GrapplerFlags::caught_player)) {
		player.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 24.f, 0.f}, true);
		player.get_collider().physics.zero();
	}
	if (m_flags.consume(GrapplerFlags::released_player)) { player.apply_impulse({directions.actual.as_float() * 10.f, -0.6f}); }
	m_grab.set_position(get_collider().get_center() + sf::Vector2f{36.f, 0.f} * directions.actual.as_float());
	m_grab.sensor.deactivate();
	if (p_animatable.animation.get_frame() == 6 || p_animatable.animation.get_frame() == 7) { m_grab.sensor.activate(); }
	if (m_grab.sensor.within_bounds(player.hurtbox) && m_grab.sensor.active() && !m_flags.test(GrapplerFlags::caught_player)) {
		player.stun(1.f);
		m_flags.set(GrapplerFlags::caught_player);
	}

	if (is_hostile() && !m_leap_cooldown.running()) { request(GrapplerState::leap); }
	if (directions.actual.lnr != directions.desired.lnr) { request(GrapplerState::turn); }

	state_function = state_function();
}

void Grappler::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (!debug::is_production() && m_grab.sensor.active()) { m_grab.render(win, cam); }
}

fsm::StateFunction Grappler::update_idle() {
	p_state.actual = GrapplerState::idle;
	if (change_state(GrapplerState::turn, get_params("turn"))) { return GRAPPLER_BIND(update_turn); }
	if (change_state(GrapplerState::leap, get_params("leap"))) { return GRAPPLER_BIND(update_leap); }
	return GRAPPLER_BIND(update_idle);
}

fsm::StateFunction Grappler::update_leap() {
	p_state.actual = GrapplerState::leap;
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * attributes.speed, -5.0f}; }
	if (m_flags.test(GrapplerFlags::caught_player)) {
		m_leap_cooldown.start();
		m_hold_time.start();
		request(GrapplerState::snag);
		if (change_state(GrapplerState::snag, get_params("snag"))) { return GRAPPLER_BIND(update_snag); }
	}
	if (p_animatable.animation.complete()) {
		m_leap_cooldown.start();
		request(GrapplerState::whiff);
		if (change_state(GrapplerState::whiff, get_params("whiff"))) { return GRAPPLER_BIND(update_whiff); }
	}
	return GRAPPLER_BIND(update_leap);
}

fsm::StateFunction Grappler::update_snag() {
	p_state.actual = GrapplerState::snag;
	shake();
	if (m_flags.test(GrapplerFlags::caught_player)) {
		if (m_hold_time.is_almost_complete()) {
			request(GrapplerState::release);
			if (change_state(GrapplerState::release, get_params("release"))) { return GRAPPLER_BIND(update_release); }
		}
	}
	return GRAPPLER_BIND(update_snag);
}

fsm::StateFunction Grappler::update_release() {
	p_state.actual = GrapplerState::release;
	if (p_animatable.animation.just_started()) {
		m_flags.reset(GrapplerFlags::caught_player);
		m_leap_cooldown.start();
		m_flags.set(GrapplerFlags::released_player);
	}
	if (p_animatable.animation.complete()) {
		request(GrapplerState::idle);
		if (change_state(GrapplerState::idle, get_params("idle"))) { return GRAPPLER_BIND(update_idle); }
	}
	return GRAPPLER_BIND(update_release);
}

fsm::StateFunction Grappler::update_whiff() {
	p_state.actual = GrapplerState::whiff;
	if (p_animatable.animation.complete()) {
		if (change_state(GrapplerState::turn, get_params("turn"))) { return GRAPPLER_BIND(update_turn); }
		request(GrapplerState::idle);
		if (change_state(GrapplerState::idle, get_params("idle"))) { return GRAPPLER_BIND(update_idle); }
	}
	return GRAPPLER_BIND(update_whiff);
}

fsm::StateFunction Grappler::update_turn() {
	p_state.actual = GrapplerState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		if (change_state(GrapplerState::leap, get_params("leap"))) { return GRAPPLER_BIND(update_leap); }
		request(GrapplerState::idle);
		if (change_state(GrapplerState::idle, get_params("idle"))) { return GRAPPLER_BIND(update_idle); }
	}
	return GRAPPLER_BIND(update_turn);
}

bool Grappler::change_state(GrapplerState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
