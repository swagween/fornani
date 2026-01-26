
#include <fornani/entities/enemy/catalog/Junkfly.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto junkfly_framerate = 12;

Junkfly::Junkfly(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "junkfly"), m_services{&svc}, m_toss_time{800} {
	m_params = {{"idle", {0, 4, junkfly_framerate * 2, -1}}, {"turn", {4, 2, junkfly_framerate * 2, 0}}, {"toss", {6, 4, junkfly_framerate * 2, 0}}};
	animation.set_params(get_params("idle"));
	p_state.actual = JunkflyState::idle;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	m_variant = static_cast<JunkflyVariant>(variant);
	m_bomb = entity::WeaponPackage{svc, "frag_grenade"};
	m_bomb->get().set_team(arms::Team::guardian);

	get_collider().physics.set_friction_componentwise({0.995f, 0.99f});

	m_toss_time.randomize();
}

void Junkfly::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_toss_time.update();

	// bomb variant stuff
	if (m_bomb) { m_bomb->update(svc, map, *this); }

	if (is_alert()) {
		m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{0.f, -160.f}, 0.00006f);
		if (m_bomb) {
			if (m_toss_time.is_complete()) { request(JunkflyState::toss); }
		}
	}

	// shoot
	if (has_flag_set(JunkflyFlags::toss) && !health.is_dead()) {
		auto bp = sf::Vector2f{0.f, 32.f};
		m_bomb->get().set_barrel_point(get_collider().get_center() + bp);
		map.spawn_projectile_at(svc, m_bomb->get(), get_collider().get_center() + bp, player.get_collider().get_center() - get_collider().get_center() + bp);
		m_toss_time.start();
		set_flag(JunkflyFlags::toss, false);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(JunkflyState::turn); }

	state_function = state_function();
}

void Junkfly::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Junkfly::update_idle() {
	p_state.actual = JunkflyState::idle;
	if (change_state(JunkflyState::turn, get_params("turn"))) { return JUNKFLY_BIND(update_turn); }
	if (change_state(JunkflyState::toss, get_params("toss"))) { return JUNKFLY_BIND(update_toss); }
	return JUNKFLY_BIND(update_idle);
}

fsm::StateFunction Junkfly::update_toss() {
	p_state.actual = JunkflyState::toss;
	if (animation.get_frame_count() == 2 && animation.keyframe_started()) {
		set_flag(JunkflyFlags::toss);
		m_services->soundboard.play_sound("missile_whistle", get_collider().get_center());
	}
	if (animation.is_complete()) {
		request(JunkflyState::idle);
		if (change_state(JunkflyState::idle, get_params("idle"))) { return JUNKFLY_BIND(update_idle); }
	}
	return JUNKFLY_BIND(update_toss);
}

fsm::StateFunction Junkfly::update_turn() {
	p_state.actual = JunkflyState::turn;
	if (animation.complete()) {
		request_flip();
		request(JunkflyState::idle);
		if (change_state(JunkflyState::idle, get_params("idle"))) { return JUNKFLY_BIND(update_idle); }
	}
	return JUNKFLY_BIND(update_turn);
}

bool Junkfly::change_state(JunkflyState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
