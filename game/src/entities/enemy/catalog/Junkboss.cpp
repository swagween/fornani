
#include <fornani/entities/enemy/catalog/Junkboss.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto junkboss_framerate = 24;

Junkboss::Junkboss(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "junkboss"), m_services{&svc}, m_toss_time{600}, m_switch_sides{2400} {
	p_animatable.set_animations({{"idle", {0, 4, junkboss_framerate * 2, -1}}, {"turn", {8, 2, junkboss_framerate * 2, 0}}, {"toss", {4, 4, junkboss_framerate, 2}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = JunkbossState::idle;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	m_variant = static_cast<JunkbossVariant>(variant);
	m_bomb = entity::WeaponPackage{svc, "frag_grenade"};
	m_bomb->get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.99f, 0.99f});

	m_toss_time.randomize();
}

void Junkboss::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_toss_time.update();
	m_switch_sides.update();
	if (m_switch_sides.is_complete()) { m_switch_sides.start(); }

	// bomb variant stuff
	if (m_bomb) { m_bomb->update(svc, map, *this); }

	if (is_alert()) {
		auto side = m_switch_sides.halfway() ? -1.f : 1.f;
		m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{150.f * side, -280.f - 80.f * std::sin(3.f * svc.ticker.total_seconds_passed.count())}, 0.00006f);
		if (m_bomb) {
			if (m_toss_time.is_complete()) { request(JunkbossState::toss); }
		}
	}

	// shoot
	if (has_flag_set(JunkbossFlags::toss) && !health.is_dead()) {
		auto bp = sf::Vector2f{directions.actual.as_float() * 10.f, 38.f};
		m_bomb->get().set_barrel_point(get_collider().get_center() + bp);
		m_bomb->get().shoot(svc, map, player.get_collider().get_center() - get_collider().get_center() + bp);
		m_toss_time.start();
		set_flag(JunkbossFlags::toss, false);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		m_services->soundboard.play_sound("demon_hurt", get_collider().get_center());
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(JunkbossState::turn); }

	state_function = state_function();
}

void Junkboss::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Junkboss::update_idle() {
	p_state.actual = JunkbossState::idle;
	if (change_state(JunkbossState::turn, get_params("turn"))) { return JUNKBOSS_BIND(update_turn); }
	if (change_state(JunkbossState::toss, get_params("toss"))) { return JUNKBOSS_BIND(update_toss); }
	return JUNKBOSS_BIND(update_idle);
}

fsm::StateFunction Junkboss::update_toss() {
	p_state.actual = JunkbossState::toss;
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) {
		set_flag(JunkbossFlags::toss);
		if (!health.is_dead()) { m_services->soundboard.play_sound("missile_whistle", get_collider().get_center()); }
	}
	if (p_animatable.animation.is_complete()) {
		request(JunkbossState::idle);
		if (change_state(JunkbossState::idle, get_params("idle"))) { return JUNKBOSS_BIND(update_idle); }
	}
	return JUNKBOSS_BIND(update_toss);
}

fsm::StateFunction Junkboss::update_turn() {
	p_state.actual = JunkbossState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(JunkbossState::idle);
		if (change_state(JunkbossState::idle, get_params("idle"))) { return JUNKBOSS_BIND(update_idle); }
	}
	return JUNKBOSS_BIND(update_turn);
}

bool Junkboss::change_state(JunkbossState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
