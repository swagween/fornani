
#include <fornani/entities/enemy/catalog/Minion.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto minion_framerate = 10;

Minion::Minion(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, "minion"), m_services{&svc}, m_jump{8}, m_tick{120} {
	m_params = {{"idle", {0, 1, minion_framerate * 2, -1}}, {"blink", {1, 2, minion_framerate * 2, 0}}, {"jump", {3, 4, minion_framerate * 2, 0}}, {"turn", {7, 1, minion_framerate * 2, 0}}};
	animation.set_params(get_params("idle"));
	flags.state.set(StateFlags::no_shake);
	flags.general.set(GeneralFlags::hurt_on_contact);
	auto random_start = random::random_range(0, m_tick.get_native_time());
	m_tick.start(random_start);

	collider.physics.set_friction_componentwise({0.96f, 0.99f});
}

void Minion::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	m_jump.update();
	m_tick.update();
	if (m_tick.is_complete()) { m_tick.start(); }

	if (m_tick.is_almost_complete()) {
		if (random::percent_chance(20)) { request(MinionState::jump); }
	}

	// caution
	auto incoming_projectile = m_caution.projectile_detected(map, physical.alert_range, arms::Team::skycorps);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != directions.actual.lnr) { request(MinionState::jump); }
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(MinionState::turn); }

	state_function = state_function();
}

void Minion::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction Minion::update_idle() {
	m_state.actual = MinionState::idle;
	if (change_state(MinionState::turn, get_params("turn"))) { return MINION_BIND(update_turn); }
	if (change_state(MinionState::blink, get_params("blink"))) { return MINION_BIND(update_blink); }
	if (collider.grounded()) {
		if (change_state(MinionState::jump, get_params("jump"))) { return MINION_BIND(update_jump); }
	}
	return MINION_BIND(update_idle);
}

fsm::StateFunction Minion::update_blink() {
	m_state.actual = MinionState::blink;
	if (animation.is_complete()) {
		if (change_state(MinionState::jump, get_params("jump"))) { return MINION_BIND(update_jump); }
		if (change_state(MinionState::turn, get_params("turn"))) { return MINION_BIND(update_turn); }
		request(MinionState::idle);
		if (change_state(MinionState::idle, get_params("idle"))) { return MINION_BIND(update_idle); }
	}
	return MINION_BIND(update_blink);
}

fsm::StateFunction Minion::update_jump() {
	m_state.actual = MinionState::jump;
	if (animation.just_started()) { m_jump.start(); }
	if (m_jump.running()) { collider.physics.acceleration.y = -18.f; }
	collider.physics.acceleration.x = directions.actual.as_float() * 2.f;
	if (animation.is_complete()) {
		request(MinionState::idle);
		if (change_state(MinionState::idle, get_params("idle"))) { return MINION_BIND(update_idle); }
	}
	return MINION_BIND(update_jump);
}

fsm::StateFunction Minion::update_turn() {
	m_state.actual = MinionState::turn;
	if (animation.complete()) {
		request_flip();
		if (change_state(MinionState::jump, get_params("jump"))) { return MINION_BIND(update_jump); }
		request(MinionState::idle);
		if (change_state(MinionState::idle, get_params("idle"))) { return MINION_BIND(update_idle); }
	}
	return MINION_BIND(update_turn);
}

bool Minion::change_state(MinionState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
