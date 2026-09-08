
#include <fornani/entities/enemy/catalog/Hivle.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto hivle_framerate = 12;

Hivle::Hivle(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "hivle"), m_services{&svc}, m_toss_time{600}, m_switch_sides{2400} {
	p_animatable.set_animations({{"idle", {0, 4, hivle_framerate * 2, 2}},
								 {"run", {4, 4, hivle_framerate * 2, 3}},
								 {"toss", {8, 7, hivle_framerate * 2, 0}},
								 {"land", {19, 2, hivle_framerate * 2, 0}},
								 {"jump", {15, 4, hivle_framerate * 3, 0, true}},
								 {"turn", {21, 2, hivle_framerate * 2, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = HivleState::idle;

	m_variant = static_cast<HivleVariant>(variant);
	m_javelin = entity::WeaponPackage{svc, "javelin"};
	m_javelin->get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.995f, 0.999f});

	m_toss_time.randomize();
}

void Hivle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_toss_time.update();
	m_switch_sides.update();
	if (m_switch_sides.is_complete()) { m_switch_sides.start(); }

	// bomb variant stuff
	if (m_javelin) { m_javelin->update(svc, map, *this); }

	// shoot
	if (has_flag_set(HivleFlags::toss) && !health.is_dead()) {
		auto bp = sf::Vector2f{directions.actual.as_float() * 10.f, -8.f};
		m_javelin->get().set_barrel_point(get_collider().get_center() + bp);
		m_javelin->get().shoot(svc, map, player.get_collider().get_center() - get_collider().get_center() + bp);
		m_toss_time.start();
		set_flag(HivleFlags::toss, false);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		m_services->soundboard.play_sound("hit_deep", get_collider().get_center());
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(HivleState::turn); }
	if (m_caution.is_projectile_detected(map, physical.alert_range, arms::Team::guardian)) { request(HivleState::jump); }

	state_function = state_function();
}

void Hivle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Hivle::update_idle() {
	p_state.actual = HivleState::idle;
	if (change_state(HivleState::turn, get_params("turn"))) { return HIVLE_BIND(update_turn); }
	if (get_collider().grounded()) {
		if (change_state(HivleState::jump, get_params("jump"))) { return HIVLE_BIND(update_jump); }
	}
	if (p_animatable.animation.is_complete()) {
		is_alert() ? request(HivleState::toss) : request(HivleState::run);
		if (change_state(HivleState::run, get_params("run"))) { return HIVLE_BIND(update_run); }
		if (change_state(HivleState::toss, get_params("toss"))) { return HIVLE_BIND(update_toss); }
	}
	return HIVLE_BIND(update_idle);
}

fsm::StateFunction Hivle::update_run() {
	p_state.actual = HivleState::run;
	get_collider().physics.velocity.x = directions.actual.as_float() * 2.f;
	if (change_state(HivleState::turn, get_params("turn"))) { return HIVLE_BIND(update_turn); }
	if (get_collider().grounded()) {
		if (change_state(HivleState::jump, get_params("jump"))) { return HIVLE_BIND(update_jump); }
	}
	if (p_animatable.animation.is_complete()) {
		is_alert() ? request(HivleState::toss) : request(HivleState::idle);
		if (change_state(HivleState::toss, get_params("toss"))) { return HIVLE_BIND(update_toss); }
		if (change_state(HivleState::idle, get_params("idle"))) { return HIVLE_BIND(update_idle); }
	}
	return HIVLE_BIND(update_run);
}

fsm::StateFunction Hivle::update_jump() {
	p_state.actual = HivleState::jump;
	if (p_animatable.frame_action(1)) { get_collider().physics.velocity.y = -14.f; }
	if (p_animatable.animation.get_frame_count() > 0) { get_collider().physics.velocity.x = directions.actual.as_float() * -2.f; }
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		request(HivleState::land);
		if (change_state(HivleState::land, get_params("land"))) { return HIVLE_BIND(update_land); }
	}
	return HIVLE_BIND(update_jump);
}

fsm::StateFunction Hivle::update_toss() {
	p_state.actual = HivleState::toss;
	if (p_animatable.frame_action(4)) { set_flag(HivleFlags::toss); }
	if (p_animatable.animation.is_complete()) {
		request(HivleState::idle);
		if (change_state(HivleState::idle, get_params("idle"))) { return HIVLE_BIND(update_idle); }
	}
	return HIVLE_BIND(update_toss);
}

fsm::StateFunction Hivle::update_land() {
	p_state.actual = HivleState::land;
	if (p_animatable.animation.complete()) {
		is_alert() ? request(HivleState::toss) : request(HivleState::run);
		if (change_state(HivleState::run, get_params("run"))) { return HIVLE_BIND(update_run); }
		if (change_state(HivleState::toss, get_params("toss"))) { return HIVLE_BIND(update_toss); }
	}
	return HIVLE_BIND(update_land);
}

fsm::StateFunction Hivle::update_turn() {
	p_state.actual = HivleState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		is_alert() ? request(HivleState::toss) : request(HivleState::run);
		if (change_state(HivleState::run, get_params("run"))) { return HIVLE_BIND(update_run); }
		if (change_state(HivleState::toss, get_params("toss"))) { return HIVLE_BIND(update_toss); }
	}
	return HIVLE_BIND(update_turn);
}

bool Hivle::change_state(HivleState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
