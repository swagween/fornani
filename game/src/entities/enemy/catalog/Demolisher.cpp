
#include <fornani/entities/enemy/catalog/Demolisher.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto demolisher_framerate = 16;

Demolisher::Demolisher(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "demolisher"), m_services{&svc}, m_jump_time{64}, m_switch_sides{2400} {
	p_animatable.set_animations({{"idle", {0, 4, demolisher_framerate * 3, 1}},
								 {"begin_run", {4, 2, demolisher_framerate * 2, 0}},
								 {"run", {6, 4, demolisher_framerate * 2, 3}},
								 {"ground_pound", {10, 9, demolisher_framerate, 2}},
								 {"turn", {19, 2, demolisher_framerate * 2, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = DemolisherState::idle;

	m_variant = static_cast<DemolisherVariant>(variant);

	m_attack.set_constant_radius(44.f);

	get_collider().physics.set_friction_componentwise({0.95f, 0.99f});
}

void Demolisher::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_jump_time.update();
	m_switch_sides.update();
	if (m_switch_sides.is_complete()) { m_switch_sides.start(); }
	if (just_died()) { m_shockwaves.clear(); }

	// spikey helmet
	m_helmet.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 8.f, -28.f});
	m_helmet.hit.activate();
	m_helmet.hurt_player(player);
	m_helmet.cancel_projectiles(svc, map, get_team(), 0.06f);

	is_state(DemolisherState::run) ? flags.general.set(GeneralFlags::hurt_on_contact) : flags.general.reset(GeneralFlags::hurt_on_contact);

	// attacks
	m_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 30.f, 40.f});
	m_attack.hit.deactivate();
	if (is_state(DemolisherState::ground_pound) && (p_animatable.animation.get_frame_count() == 4 || p_animatable.animation.get_frame_count() == 5)) { m_attack.hit.activate(); }
	m_attack.hurt_player(player, 2.f, {Enemy::directions.desired.as_float() * 0.2f, -0.2f});

	// shockwaves
	for (auto& s : m_shockwaves) {
		if (s.hit.active()) { player.hurt(); }
		s.update(svc, map);
		s.handle_player(player);
	}
	std::erase_if(m_shockwaves, [](auto const& s) { return s.lifetime.is_almost_complete(); });

	if (m_caution.detected_step(map, get_collider(), directions.actual)) { set_flag(DemolisherFlags::step_detected); }

	if (directions.actual.lnr != directions.desired.lnr) { request(DemolisherState::turn); }
	if (m_caution.is_projectile_detected(map, physical.hostile_range, arms::Team::guardian)) { request(DemolisherState::begin_run); }

	state_function = state_function();
}

void Demolisher::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	// m_attack.render(win, cam);
	// m_helmet.render(win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Demolisher::update_idle() {
	p_state.actual = DemolisherState::idle;
	if (change_state(DemolisherState::turn, get_params("turn"))) { return DEMOLISHER_BIND(update_turn); }
	if (p_animatable.animation.is_complete()) {
		is_hostile() ? request(DemolisherState::ground_pound) : is_alert() ? request(DemolisherState::begin_run) : request(DemolisherState::idle);
		if (change_state(DemolisherState::idle, get_params("idle"))) { return DEMOLISHER_BIND(update_idle); }
		if (change_state(DemolisherState::ground_pound, get_params("ground_pound"))) { return DEMOLISHER_BIND(update_ground_pound); }
		if (change_state(DemolisherState::begin_run, get_params("begin_run"))) { return DEMOLISHER_BIND(update_begin_run); }
	}
	return DEMOLISHER_BIND(update_idle);
}

fsm::StateFunction Demolisher::update_begin_run() {
	p_state.actual = DemolisherState::begin_run;
	if (p_animatable.animation.complete()) {
		request(DemolisherState::run);
		if (change_state(DemolisherState::run, get_params("run"))) { return DEMOLISHER_BIND(update_run); }
	}
	return DEMOLISHER_BIND(update_begin_run);
}

fsm::StateFunction Demolisher::update_run() {
	p_state.actual = DemolisherState::run;
	get_collider().physics.velocity.x = directions.actual.as_float() * 8.f;
	if (change_state(DemolisherState::turn, get_params("turn"))) { return DEMOLISHER_BIND(update_turn); }
	if (p_animatable.animation.is_complete()) {
		is_alert() ? request(DemolisherState::ground_pound) : request(DemolisherState::idle);
		if (change_state(DemolisherState::ground_pound, get_params("ground_pound"))) { return DEMOLISHER_BIND(update_ground_pound); }
		if (change_state(DemolisherState::idle, get_params("idle"))) { return DEMOLISHER_BIND(update_idle); }
	}
	return DEMOLISHER_BIND(update_run);
}

fsm::StateFunction Demolisher::update_ground_pound() {
	p_state.actual = DemolisherState::ground_pound;
	if (p_animatable.frame_action(4)) {
		m_services->soundboard.play_sound("heavy_land", get_collider().get_center());
		m_services->camera_controller.shake(10, 0.2f, 200, 20);
		m_shockwaves.push_back(entity::Shockwave{{20, 500, 3, {1.5f * directions.actual.as_float(), 0.f}}});
		m_shockwaves.back().origin = Enemy::get_collider().get_bottom();
		m_shockwaves.back().start();
	}
	if (p_animatable.animation.complete()) {
		request(DemolisherState::idle);
		if (change_state(DemolisherState::idle, get_params("idle"))) { return DEMOLISHER_BIND(update_idle); }
	}
	return DEMOLISHER_BIND(update_ground_pound);
}

fsm::StateFunction Demolisher::update_turn() {
	p_state.actual = DemolisherState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		is_alert() ? request(DemolisherState::ground_pound) : request(DemolisherState::begin_run);
		if (change_state(DemolisherState::ground_pound, get_params("ground_pound"))) { return DEMOLISHER_BIND(update_ground_pound); }
		if (change_state(DemolisherState::begin_run, get_params("begin_run"))) { return DEMOLISHER_BIND(update_begin_run); }
	}
	return DEMOLISHER_BIND(update_turn);
}

bool Demolisher::change_state(DemolisherState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
