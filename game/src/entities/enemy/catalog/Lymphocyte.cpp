
#include <fornani/entities/enemy/catalog/Lymphocyte.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto lymphocyte_framerate = 20;

Lymphocyte::Lymphocyte(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "lymphocyte"), m_services{&svc}, m_map{&map} {
	p_animatable.set_animations({{"dormant", {0, 1, lymphocyte_framerate, -1}},
								 {"spawn", {0, 3, lymphocyte_framerate, 0}},
								 {"idle", {3, 4, lymphocyte_framerate * 2, -1}},
								 {"make_antibody", {7, 6, lymphocyte_framerate * 2, 0}},
								 {"turn", {13, 2, lymphocyte_framerate, 0}}});
	p_animatable.animation.set_params(get_params("dormant"));
	p_state.actual = LymphocyteState::dormant;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	get_collider().physics.set_friction_componentwise({0.98f, 0.9f});
}

void Lymphocyte::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { spawn_antibody(3); }
	Enemy::update(svc, map, player);
	if (physical.hostile_range.overlaps(player.hurtbox)) {
		m_flags.set(LymphocyteFlags::alerted);
		request(LymphocyteState::spawn);
		state_function = state_function();
	}
	if (!m_flags.test(LymphocyteFlags::alerted)) { return; }

	flags.state.set(StateFlags::vulnerable);
	if (died()) { return; }

	if (is_alert()) {
		face_player(player);
		m_steering.thrust_seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{0.f, -8.f}, {0.006f, .118f, .991f, 60.f});
	} else {
		face_movement();
		m_steering.smooth_random_walk(Enemy::get_collider().physics, 0.005f, 64.f);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
		svc.soundboard.play_sound("hit");
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(LymphocyteState::turn); }
	if (is_alert() && svc.ticker.every_x_ticks(700)) { request(LymphocyteState::make_antibody); }

	state_function = state_function();
}

void Lymphocyte::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (!m_flags.test(LymphocyteFlags::alerted)) { return; }
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Lymphocyte::update_dormant() {
	p_state.actual = LymphocyteState::dormant;
	if (change_state(LymphocyteState::spawn, get_params("spawn"))) { return LYMPHOCYTE_BIND(update_spawn); }
	return LYMPHOCYTE_BIND(update_dormant);
}

fsm::StateFunction Lymphocyte::update_spawn() {
	p_state.actual = LymphocyteState::spawn;
	if (p_animatable.animation.is_complete()) {
		if (change_state(LymphocyteState::turn, get_params("turn"))) { return LYMPHOCYTE_BIND(update_turn); }
		request(LymphocyteState::idle);
		if (change_state(LymphocyteState::idle, get_params("idle"))) { return LYMPHOCYTE_BIND(update_idle); }
	}
	return LYMPHOCYTE_BIND(update_spawn);
}

fsm::StateFunction Lymphocyte::update_idle() {
	p_state.actual = LymphocyteState::idle;
	if (change_state(LymphocyteState::make_antibody, get_params("make_antibody"))) { return LYMPHOCYTE_BIND(update_make_antibody); }
	if (change_state(LymphocyteState::turn, get_params("turn"))) { return LYMPHOCYTE_BIND(update_turn); }
	return LYMPHOCYTE_BIND(update_idle);
}

fsm::StateFunction Lymphocyte::update_make_antibody() {
	p_state.actual = LymphocyteState::make_antibody;
	if (p_animatable.frame_action(3)) {
		spawn_antibody(1);
		m_services->soundboard.play_sound("beast_spawn", get_collider().get_center());
	}
	if (p_animatable.animation.complete()) {
		request(LymphocyteState::idle);
		if (change_state(LymphocyteState::idle, get_params("idle"))) { return LYMPHOCYTE_BIND(update_idle); }
	}
	return LYMPHOCYTE_BIND(update_make_antibody);
}

fsm::StateFunction Lymphocyte::update_turn() {
	p_state.actual = LymphocyteState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(LymphocyteState::idle);
		if (change_state(LymphocyteState::idle, get_params("idle"))) { return LYMPHOCYTE_BIND(update_idle); }
	}
	return LYMPHOCYTE_BIND(update_turn);
}

bool Lymphocyte::change_state(LymphocyteState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

void Lymphocyte::spawn_antibody(int amount) {
	for (int i = 0; i < amount; ++i) {
		auto variant = random::percent_chance(20) ? 1 : 0;
		m_map->spawn_enemy(37, get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 8.f, 0.f}, variant, true, false);
	}
}

} // namespace fornani::enemy
