
#include <fornani/entities/enemy/catalog/Junker.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto junker_framerate = 12;

Junker::Junker(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "junker"), m_services{&svc}, m_toss_time{400} {
	m_params = {{"idle", {0, 8, junker_framerate * 2, -1}}, {"turn", {12, 2, junker_framerate * 2, 0}}, {"toss", {8, 4, junker_framerate * 3, 0}}, {"hide", {14, 1, junker_framerate * 2, -1}}};
	animation.set_params(get_params("hide"));
	p_state.actual = JunkerState::hide;

	m_variant = static_cast<JunkerVariant>(variant);
	m_bomb = entity::WeaponPackage{svc, "frag_grenade"};
	m_bomb->get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.995f, 0.99f});

	m_toss_time.randomize();
}

void Junker::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	face_player(player);
	is_state(JunkerState::hide) ? flags.state.reset(StateFlags::vulnerable) : flags.state.set(StateFlags::vulnerable);

	m_toss_time.update();

	// bomb variant stuff
	if (m_bomb) { m_bomb->update(svc, map, *this); }

	if (is_hostile()) {
		if (m_bomb) {
			if (m_toss_time.is_complete()) { request(JunkerState::toss); }
		}
	}

	// shoot
	if (has_flag_set(JunkerFlags::toss) && !health.is_dead()) {
		auto sign = directions.actual.as_float();
		auto bp = sf::Vector2f{sign * 18.f, -18.f};
		m_bomb->get().set_barrel_point(get_collider().get_center() + bp);
		m_bomb->get().shoot(svc, map, player.get_collider().get_center() - get_collider().get_center() + bp);
		m_toss_time.start();
		set_flag(JunkerFlags::toss, false);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	// caution
	auto incoming_projectile = m_caution.projectile_detected(map, physical.alert_range, arms::Team::beast);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != directions.actual.lnr) { request(JunkerState::hide); }
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(JunkerState::turn); }

	state_function = state_function();
}

void Junker::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Junker::update_idle() {
	p_state.actual = JunkerState::idle;
	if (change_state(JunkerState::hide, get_params("hide"))) { return JUNKER_BIND(update_hide); }
	if (change_state(JunkerState::turn, get_params("turn"))) { return JUNKER_BIND(update_turn); }
	if (change_state(JunkerState::toss, get_params("toss"))) { return JUNKER_BIND(update_toss); }
	return JUNKER_BIND(update_idle);
}

fsm::StateFunction Junker::update_toss() {
	p_state.actual = JunkerState::toss;
	if (animation.get_frame_count() == 2 && animation.keyframe_started()) { set_flag(JunkerFlags::toss); }
	if (animation.is_complete()) {
		request(JunkerState::hide);
		if (change_state(JunkerState::hide, get_params("hide"))) { return JUNKER_BIND(update_hide); }
	}
	return JUNKER_BIND(update_toss);
}

fsm::StateFunction Junker::update_turn() {
	p_state.actual = JunkerState::turn;
	if (animation.complete()) {
		request_flip();
		request(JunkerState::hide);
		if (change_state(JunkerState::hide, get_params("hide"))) { return JUNKER_BIND(update_hide); }
	}
	return JUNKER_BIND(update_turn);
}

fsm::StateFunction Junker::update_hide() {
	p_state.actual = JunkerState::hide;
	if (directions.actual.lnr != directions.desired.lnr) {
		flip();
		directions.actual = directions.desired;
	}
	if (change_state(JunkerState::toss, get_params("toss"))) { return JUNKER_BIND(update_toss); }
	return JUNKER_BIND(update_hide);
}

bool Junker::change_state(JunkerState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
