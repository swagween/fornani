
#include "fornani/entities/enemy/catalog/Hulmet.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Hulmet::Hulmet(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "hulmet"), m_services(&svc), m_map(&map), m_parts{.gun{svc.assets.get_texture("hulmet_gun"), 2.0f, 0.85f, {-16.f, 8.f}}} {};

void Hulmet::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) {
		return;
	}
	if (directions.actual.lnr != directions.desired.lnr) { request(HulmetState::turn); }
	state_function = state_function();
}

void Hulmet::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {}

fsm::StateFunction Hulmet::update_idle() {
	animation.label = "idle";
	m_state.actual = HulmetState::idle;
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::sleep, m_animations.sleep)) { return HULMET_BIND(update_sleep); }
	if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	return HULMET_BIND(update_idle);
}

fsm::StateFunction Hulmet::update_turn() {
	animation.label = "turn";
	m_state.actual = HulmetState::turn;
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::idle, m_animations.idle)) { return HULMET_BIND(update_idle); }
	if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	return HULMET_BIND(update_turn);
}

fsm::StateFunction Hulmet::update_run() {
	animation.label = "run";
	m_state.actual = HulmetState::run;
	if (change_state(HulmetState::turn, m_animations.turn)) { return HULMET_BIND(update_turn); }
	if (change_state(HulmetState::idle, m_animations.idle)) { return HULMET_BIND(update_idle); }
	if (change_state(HulmetState::alert, m_animations.alert)) { return HULMET_BIND(update_alert); }
	if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	if (change_state(HulmetState::jump, m_animations.jump)) { return HULMET_BIND(update_jump); }
	return HULMET_BIND(update_run);
}

fsm::StateFunction Hulmet::update_alert() {
	animation.label = "alert";
	m_state.actual = HulmetState::alert;
	if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	if (change_state(HulmetState::jump, m_animations.jump)) { return HULMET_BIND(update_jump); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	return HULMET_BIND(update_alert);
}

fsm::StateFunction Hulmet::update_roll() {
	animation.label = "roll";
	m_state.actual = HulmetState::roll;
	if (change_state(HulmetState::jump, m_animations.jump)) { return HULMET_BIND(update_jump); }
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	return HULMET_BIND(update_roll);
}

fsm::StateFunction Hulmet::update_jump() {
	animation.label = "jump";
	m_state.actual = HulmetState::jump;
	if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	return HULMET_BIND(update_jump);
}

fsm::StateFunction Hulmet::update_shoot() {
	animation.label = "shoot";
	m_state.actual = HulmetState::shoot;
	if (change_state(HulmetState::shoot, m_animations.shoot)) { return HULMET_BIND(update_shoot); }
	if (change_state(HulmetState::roll, m_animations.roll)) { return HULMET_BIND(update_roll); }
	if (change_state(HulmetState::run, m_animations.run)) { return HULMET_BIND(update_run); }
	if (change_state(HulmetState::idle, m_animations.idle)) { return HULMET_BIND(update_idle); }
	return HULMET_BIND(update_shoot);
}

fsm::StateFunction Hulmet::update_sleep() {
	animation.label = "sleep";
	m_state.actual = HulmetState::sleep;
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

}
