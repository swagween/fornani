
#include <fornani/entities/enemy/catalog/Spewer.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Spewer::Spewer(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "spewer"), m_services{&svc}, m_map{&map}, m_spew{svc, "toxic_spew"} {
	p_animatable.set_animations({{"idle", {0, 2, 40, -1}}, {"walk", {2, 4, 32, 3}}, {"prepare", {6, 4, 32, 0}}, {"spew", {10, 2, 32, 5}}, {"turn", {12, 1, 32, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = SpewerState::idle;
	get_collider().physics.set_friction_componentwise({0.95f, 0.98f});
	flags.state.set(StateFlags::vulnerable);
	attributes.team = arms::Team::beast;
	m_spew.set_team(arms::Team::beast);
}

void Spewer::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	// spew
	auto bp = sf::Vector2f{0.f, -80.f};
	m_spew.get().set_barrel_point(get_collider().get_center() + bp);
	m_spew.get().set_firing_direction(CardinalDirection{UDLR::up});
	m_spew.update(svc, map, *this);

	if (svc.ticker.every_second() && random::percent_chance(20)) { request(SpewerState::prepare); }
	if (directions.actual.lnr != directions.desired.lnr) { request(SpewerState::turn); }

	state_function = state_function();
}

void Spewer::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction Spewer::update_idle() {
	p_state.actual = SpewerState::idle;
	if (change_state(SpewerState::turn, get_params("turn"))) { return SPEWER_BIND(update_turn); }
	if (change_state(SpewerState::prepare, get_params("prepare"))) { return SPEWER_BIND(update_prepare); }
	return SPEWER_BIND(update_idle);
}

fsm::StateFunction Spewer::update_walk() {
	p_state.actual = SpewerState::walk;
	get_collider().physics.velocity.x = attributes.speed * directions.actual.as_float();
	if (p_animatable.animation.complete()) {
		request(SpewerState::idle);
		if (change_state(SpewerState::idle, get_params("idle"))) { return SPEWER_BIND(update_idle); }
	}
	return SPEWER_BIND(update_walk);
}

fsm::StateFunction Spewer::update_prepare() {
	p_state.actual = SpewerState::prepare;
	if (p_animatable.animation.complete()) {
		request(SpewerState::spew);
		if (change_state(SpewerState::spew, get_params("spew"))) { return SPEWER_BIND(update_spew); }
	}
	return SPEWER_BIND(update_prepare);
}

fsm::StateFunction Spewer::update_spew() {
	p_state.actual = SpewerState::spew;
	if (m_services->ticker.every_x_ticks(30)) { m_spew.shoot(*m_services, *m_map); }
	if (p_animatable.animation.complete()) {
		request(SpewerState::walk);
		if (change_state(SpewerState::walk, get_params("walk"))) { return SPEWER_BIND(update_walk); }
	}
	return SPEWER_BIND(update_spew);
}

fsm::StateFunction Spewer::update_turn() {
	p_state.actual = SpewerState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(SpewerState::idle);
		if (change_state(SpewerState::idle, get_params("idle"))) { return SPEWER_BIND(update_idle); }
	}
	return SPEWER_BIND(update_turn);
}

bool Spewer::change_state(SpewerState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
