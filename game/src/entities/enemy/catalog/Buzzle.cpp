
#include <fornani/entities/enemy/catalog/Buzzle.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto buzzle_framerate = 18;

Buzzle::Buzzle(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "buzzle"), m_services{&svc}, m_dive_timer{120}, m_recovery{400} {
	p_animatable.set_animations({{"idle", {0, 4, buzzle_framerate, -1}}, {"turn", {9, 2, buzzle_framerate, 0}}, {"signal", {4, 3, buzzle_framerate * 3, 0}}, {"divebomb", {7, 2, buzzle_framerate, 0, true}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = BuzzleState::idle;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	get_collider().physics.set_friction_componentwise({0.98f, 0.9f});
}

void Buzzle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	flags.state.set(StateFlags::vulnerable);
	m_dive_timer.update();
	m_recovery.update();

	if (is_alert() && !is_state(BuzzleState::signal)) {
		face_player(player);
		m_steering.thrust_seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{0.f, -8.f}, ThrustParameters{0.015f, .218f, .999, 40.f});
	} else {
		face_movement();
		m_steering.smooth_random_walk(Enemy::get_collider().physics, 0.005f, 64.f);
	}
	if (!is_state(BuzzleState::divebomb)) { m_target = player.hurtbox.get_center() - get_collider().get_center(); }

	if (directions.actual.lnr != directions.desired.lnr) { request(BuzzleState::turn); }

	if (is_hostile() && !m_recovery.running()) { request(BuzzleState::signal); }

	state_function = state_function();
}

void Buzzle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Buzzle::update_idle() {
	p_state.actual = BuzzleState::idle;
	if (change_state(BuzzleState::signal, get_params("signal"))) { return BUZZLE_BIND(update_signal); }
	if (change_state(BuzzleState::turn, get_params("turn"))) { return BUZZLE_BIND(update_turn); }
	return BUZZLE_BIND(update_idle);
}

fsm::StateFunction Buzzle::update_turn() {
	p_state.actual = BuzzleState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(BuzzleState::idle);
		if (change_state(BuzzleState::idle, get_params("idle"))) { return BUZZLE_BIND(update_idle); }
	}
	return BUZZLE_BIND(update_turn);
}

fsm::StateFunction Buzzle::update_signal() {
	p_state.actual = BuzzleState::signal;
	if (p_animatable.animation.complete()) {
		m_dive_timer.start();
		request(BuzzleState::divebomb);
		if (change_state(BuzzleState::divebomb, get_params("divebomb"))) { return BUZZLE_BIND(update_divebomb); }
	}
	return BUZZLE_BIND(update_signal);
}

fsm::StateFunction Buzzle::update_divebomb() {
	p_state.actual = BuzzleState::divebomb;
	get_collider().physics.velocity = m_target.normalized() * 2.5f;
	if (m_dive_timer.is_almost_complete()) {
		m_recovery.start();
		request(BuzzleState::idle);
		if (change_state(BuzzleState::idle, get_params("idle"))) { return BUZZLE_BIND(update_idle); }
	}
	return BUZZLE_BIND(update_divebomb);
}

bool Buzzle::change_state(BuzzleState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
