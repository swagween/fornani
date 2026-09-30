
#include <fornani/entities/enemy/catalog/Fiend.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto fiend_framerate = 12;

Fiend::Fiend(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "fiend"), m_services{&svc}, m_jump_time{64}, m_post_attack{200}, m_post_sidestep{600}, m_teleport{200}, m_map{&map} {
	p_animatable.set_animations({{"idle", {0, 6, fiend_framerate * 2, -1}},
								 {"prepare_sweep", {6, 5, fiend_framerate * 4, 0}},
								 {"sweep", {11, 7, fiend_framerate * 2, 0}},
								 {"prepare_uppercut", {18, 4, fiend_framerate * 4, 0}},
								 {"uppercut", {22, 5, fiend_framerate * 2, 0}},
								 {"sidestep", {27, 1, fiend_framerate, 0}},
								 {"turn", {28, 14, fiend_framerate * 2, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = FiendState::idle;

	m_variant = static_cast<FiendVariant>(variant);

	get_collider().physics.set_friction_componentwise({0.95f, 0.999f});
}

void Fiend::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) { svc.soundboard.play_sound("beast_damage", get_collider().get_center()); }
	Enemy::update(svc, map, player);
	if (health.is_dead()) { return; }
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_jump_time.update();
	m_post_attack.update();
	m_post_sidestep.update();
	m_teleport.update();

	m_player_position = player.get_collider().get_center();
	// if (m_post_sidestep.is_almost_complete()) { teleport(); }
	if (m_teleport.is_almost_complete()) {
		if (m_teleport_position) {
			get_collider().set_position(*m_teleport_position);
			m_teleport_position.reset();

			m_map->spawn_effect(*m_services, "explosion", get_collider().get_center(), {}, 5);
			flags.state.reset(StateFlags::invisible);
		}
	}

	// attacks
	m_second_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 18.f, 0.f});
	m_attack.hit.deactivate();
	m_second_attack.hit.deactivate();
	if (p_animatable.animation.get_frame() == 11) {
		m_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 40.f, -20.f});
		m_attack.set_constant_radius(96.f);
		m_attack.hit.activate();
	}
	if (p_animatable.animation.get_frame() == 14) {
		m_attack.set_position(get_collider().get_center());
		m_second_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -48.f, 0.f});
		m_second_attack.set_constant_radius(40.f);
		m_second_attack.hit.activate();
	}
	if (p_animatable.animation.get_frame() == 22) {
		m_attack.set_position(get_collider().get_center());
		m_attack.set_constant_radius(130.f);
		m_attack.hit.activate();
	}
	if (p_animatable.animation.get_frame() >= 31 && p_animatable.animation.get_frame() <= 37) {
		m_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -18.f, 0.f});
		auto const sign = p_animatable.animation.get_frame() % 2 == 0 ? -1.f : 1.f;
		m_second_attack.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -68.f * sign, -8.f});
		m_attack.set_constant_radius(46.f);
		m_second_attack.set_constant_radius(30.f);
		m_attack.hit.activate();
		m_second_attack.hit.activate();
	}
	m_attack.hurt_player(player, 2.f, {Enemy::directions.desired.as_float() * 0.2f, -0.2f});
	m_attack.cancel_projectiles(svc, map, get_team(), 0.06f);
	m_second_attack.hurt_player(player, 2.f, {Enemy::directions.desired.as_float() * 0.2f, -0.2f});
	m_second_attack.cancel_projectiles(svc, map, get_team(), 0.06f);

	if (is_hostile() && !m_post_attack.running()) {
		request(FiendState::prepare_sweep);
		if (player.get_collider().get_center().y < get_collider().get_center().y) { request(FiendState::prepare_uppercut); }
	}
	if (has_flag_set(FiendFlags::projectile) && !health.is_dead()) {
		auto bp = sf::Vector2f{directions.actual.as_float() * 10.f, -8.f};
		set_flag(FiendFlags::projectile, false);
	}
	if (m_caution.detected_step(map, get_collider(), directions.actual)) { set_flag(FiendFlags::step_detected); }

	if (directions.actual.lnr != directions.desired.lnr) { request(FiendState::turn); }
	if (m_caution.is_projectile_detected(map, physical.hostile_range, arms::Team::guardian) && !m_post_sidestep.running()) { request(FiendState::sidestep); }

	state_function = state_function();
}

void Fiend::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	// if (m_attack.hit.active()) { m_attack.render(win, cam); }
	// if (m_second_attack.hit.active()) { m_second_attack.render(win, cam); }
	if (health.is_dead()) { return; }
}

fsm::StateFunction Fiend::update_idle() {
	p_state.actual = FiendState::idle;
	if (change_state(FiendState::turn, get_params("turn"))) { return FIEND_BIND(update_turn); }
	if (get_collider().grounded()) {
		if (change_state(FiendState::sidestep, get_params("sidestep"))) { return FIEND_BIND(update_sidestep); }
	}
	if (change_state(FiendState::prepare_sweep, get_params("prepare_sweep"))) { return FIEND_BIND(update_prepare_sweep); }
	if (change_state(FiendState::prepare_uppercut, get_params("prepare_uppercut"))) { return FIEND_BIND(update_prepare_uppercut); }
	return FIEND_BIND(update_idle);
}

fsm::StateFunction Fiend::update_prepare_sweep() {
	p_state.actual = FiendState::prepare_sweep;
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity.x = directions.actual.as_float() * 30.f; }
	if (p_animatable.animation.is_complete()) {
		request(FiendState::sweep);
		if (change_state(FiendState::sweep, get_params("sweep"))) { return FIEND_BIND(update_sweep); }
	}
	return FIEND_BIND(update_prepare_sweep);
}

fsm::StateFunction Fiend::update_sidestep() {
	p_state.actual = FiendState::sidestep;
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity.y = -4.f; }
	get_collider().physics.velocity.x = directions.actual.as_float() * -16.f;
	if (p_animatable.animation.complete()) {
		m_post_sidestep.start();
		request(FiendState::idle);
		if (change_state(FiendState::idle, get_params("idle"))) { return FIEND_BIND(update_idle); }
	}
	return FIEND_BIND(update_sidestep);
}

fsm::StateFunction Fiend::update_prepare_uppercut() {
	p_state.actual = FiendState::prepare_uppercut;
	if (p_animatable.animation.get_frame_count() > 1) { shake(); }
	if (p_animatable.animation.is_complete()) {
		request(FiendState::uppercut);
		if (change_state(FiendState::uppercut, get_params("uppercut"))) { return FIEND_BIND(update_uppercut); }
	}
	return FIEND_BIND(update_prepare_uppercut);
}

fsm::StateFunction Fiend::update_sweep() {
	p_state.actual = FiendState::sweep;
	if (p_animatable.animation.is_complete()) {
		m_post_attack.start();
		request(FiendState::idle);
		if (change_state(FiendState::idle, get_params("idle"))) { return FIEND_BIND(update_idle); }
	}
	return FIEND_BIND(update_sweep);
}

fsm::StateFunction Fiend::update_uppercut() {
	p_state.actual = FiendState::uppercut;
	if (p_animatable.animation.is_complete()) {
		m_post_attack.start();
		request(FiendState::idle);
		if (change_state(FiendState::idle, get_params("idle"))) { return FIEND_BIND(update_idle); }
	}
	return FIEND_BIND(update_uppercut);
}

fsm::StateFunction Fiend::update_turn() {
	p_state.actual = FiendState::turn;
	if (p_animatable.animation.get_frame_count() > 2 && p_animatable.animation.get_frame_count() < 10) { get_collider().physics.velocity.x = directions.actual.as_float() * -6.f; }
	if (p_animatable.animation.complete()) {
		request_flip();
		request(FiendState::idle);
		if (change_state(FiendState::idle, get_params("idle"))) { return FIEND_BIND(update_idle); }
	}
	return FIEND_BIND(update_turn);
}

void Fiend::teleport() {
	if (auto position = find_teleport_position()) {
		m_teleport_position = position;
		m_teleport.start();
		flags.state.set(StateFlags::invisible);
		m_map->spawn_effect(*m_services, "explosion", get_collider().get_center(), {}, 4);
	}
}

std::optional<sf::Vector2f> Fiend::find_teleport_position() {
	constexpr float teleport_distance = 256.f;

	auto const fiend_x = get_collider().get_center().x;
	auto const player_x = m_player_position.x;
	auto const direction = player_x > fiend_x ? 1.f : -1.f;

	auto const try_position = [&](float x) -> std::optional<sf::Vector2f> {
		auto position = get_collider().get_position();
		position.x = x - get_collider().bounding_box.get_dimensions().x * 0.5f;

		for (int i{}; i < 32; ++i) {
			if (!m_map->overlaps_middleground(position)) { return position; }

			position.y -= 4.f;
		}

		return std::nullopt;
	};

	if (auto position = try_position(player_x - direction * teleport_distance)) { return position; }

	return try_position(player_x + direction * teleport_distance);
}

bool Fiend::change_state(FiendState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
