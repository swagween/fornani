
#include <fornani/entities/enemy/boss/Sludgelord.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto sludgelord_framerate = 12;

Sludgelord::Sludgelord(automa::ServiceProvider& svc, world::Map& map) : Boss(svc, map, "sludgelord"), m_services{&svc}, m_spit_time{600}, m_switch_sides{1900}, m_spew{svc, "toxic_spew"}, m_slam{6} {
	p_animatable.set_animations({{"idle", {0, 4, sludgelord_framerate * 4, -1}},
								 {"turn", {4, 2, sludgelord_framerate * 4, 0}},
								 {"spit", {6, 9, sludgelord_framerate * 3, 0}},
								 {"bounce", {15, 5, sludgelord_framerate * 2, 6}},
								 {"roll", {15, 5, sludgelord_framerate, 4}},
								 {"roar", {18, 1, sludgelord_framerate * 20, 0}},
								 {"spew", {15, 5, sludgelord_framerate, 8}},
								 {"slam", {19, 7, sludgelord_framerate, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = SludgelordState::idle;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	m_bomb = entity::WeaponPackage{svc, "poison_ball"};
	attributes.team = arms::Team::beast;
	m_bomb->get().set_team(arms::Team::beast);
	m_spew.set_team(arms::Team::beast);

	get_collider().physics.set_friction_componentwise({0.995f, 0.99f});
	m_bounce_collider.physics.elasticity = 1.f;
	m_bounce_collider.physics.set_friction_componentwise({0.999f, 0.999f});
}

void Sludgelord::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Boss::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);
	m_spit_time.update();
	m_switch_sides.update();
	m_slam.update();
	if (m_switch_sides.is_complete()) { m_switch_sides.start(); }
	if (m_slam.is_almost_complete()) {
		auto const shockwave_speed = 2.6f;
		auto const offset = sf::Vector2f{30.f, 0.f};
		m_shockwaves.push_back(entity::Shockwave{{12, 600, 6, {shockwave_speed * directions.actual.as_float(), 0.f}}});
		m_shockwaves.back().origin = Enemy::get_collider().get_bottom() + offset;
		m_shockwaves.back().start();
		m_shockwaves.push_back(entity::Shockwave{{12, 600, 6, {-shockwave_speed * directions.actual.as_float(), 0.f}}});
		m_shockwaves.back().origin = Enemy::get_collider().get_bottom() - offset;
		m_shockwaves.back().start();
	}

	if (consume_flag(BossFlags::start_battle)) {
		svc.music_player.load(svc.finder, "nimbus_iii");
		svc.music_player.play_looped();
		svc.data.switch_destructible_state(101402, true);
	}
	if (has_flag_set(BossFlags::end_battle) && !has_flag_set(BossFlags::post_death)) {
		svc.data.switch_destructible_state(101401);
		svc.music_player.pause();
		set_flag(BossFlags::post_death);
		svc.music_player.load(svc.finder, "hellions");
		svc.music_player.play_looped();
	}

	if (!health.is_dead()) { svc.soundboard.repeat_sound("sludgelord_passive", 1U, get_collider().get_center()); }

	if (is_alert() && !battle_mode()) { start_battle(); }

	// logic
	if (m_bomb) { m_bomb->update(svc, map, *this); }
	auto bp = sf::Vector2f{0.f, -48.f};
	m_spew.get().set_barrel_point(get_collider().get_center() + bp);
	m_spew.get().set_firing_direction(CardinalDirection{UDLR::up});
	m_spew.update(svc, map, *this);
	for (auto& s : m_shockwaves) {
		if (s.hit.active()) { player.hurt(); }
		s.update(svc, map);
		s.handle_player(player);
	}
	std::erase_if(m_shockwaves, [](auto const& s) { return s.lifetime.is_almost_complete(); });

	// puff effect
	if (svc.ticker.every_x_ticks(50)) { map.spawn_effect(svc, "toxic_puff", get_collider().get_center() + sf::Vector2f{0.f, -60.f} + random::vector_float(-30.f, 30.f), {0.f, -0.5f}); }

	if (battle_mode()) {
		auto side = m_switch_sides.halfway() ? -1.f : 1.f;
		if (!m_flags.test(SludgelordFlags::slamming)) {
			auto target = sf::Vector2f{150.f * side, -220.f - 80.f * std::sin(3.f * svc.ticker.total_seconds_passed.count())};
			if (is_state(SludgelordState::roll)) { target.x = 0.f; }
			if (is_state(SludgelordState::spew) || is_state(SludgelordState::roar)) {
				get_collider().physics.set_friction_componentwise({0.97f, 0.97f});
				m_steering.seek(Enemy::get_collider().physics, map.get_nearest_target_point(get_collider().get_center()), 0.0005f);
			} else {
				get_collider().physics.set_friction_componentwise({0.995f, 0.99f});
				m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center() + target, 0.00006f);
			}
		}
		if (m_spit_time.is_complete()) {
			random::coin_flip() || m_flags.test(SludgelordFlags::bounced) ? request(SludgelordState::spit) : request(SludgelordState::bounce);
			if (half_health() && !m_flags.test(SludgelordFlags::bounced)) {
				if (random::coin_flip()) { request(SludgelordState::roll); }
			}
		}
	}

	// shoot
	if (m_flags.test(SludgelordFlags::spit) && !health.is_dead() && m_bomb) {
		auto bp = sf::Vector2f{directions.actual.as_float() * 38.f, 28.f};
		m_bomb->get().set_barrel_point(get_collider().get_center() + bp);
		m_bomb->get().shoot(svc, map, player.get_collider().get_center() - get_collider().get_center() + bp);
		m_spit_time.start();
		m_flags.reset(SludgelordFlags::spit);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(SludgelordState::turn); }

	m_bounce_collider.physics.acceleration = {};

	state_function = state_function();

	m_bounce_collider.set_attribute(shape::ColliderAttributes::no_collision);
	if (m_flags.test(SludgelordFlags::slamming)) {
		flags.general.set(GeneralFlags::gravity);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
		if (!get_collider().grounded()) { get_collider().physics.velocity.y = 60.f; }
		flags.general.set(GeneralFlags::player_collision);
		flags.general.set(GeneralFlags::map_collision);
		m_bounce_collider.set_position(get_collider().get_center());
	} else if (is_state(SludgelordState::bounce)) {
		set_position(m_bounce_collider.get_global_center() - get_collider().bounding_box.get_dimensions() * 0.5f);
		m_bounce_collider.set_attribute(shape::ColliderAttributes::no_collision, false);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
		auto const dir = player_behind(player) ? -1.f : 1.f;
		m_bounce_collider.physics.apply_force({dir * 0.5f, 1.5f});
		flags.general.reset(GeneralFlags::map_collision);
	} else {
		flags.general.reset(GeneralFlags::gravity);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
		flags.general.reset(GeneralFlags::player_collision);
		m_bounce_collider.set_position(get_collider().get_center());
		flags.general.set(GeneralFlags::map_collision);
	}
	m_bounce_collider.update(svc);
	m_bounce_collider.handle_map_collision(map);
}

void Sludgelord::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	if (health.is_dead()) { return; }
	// m_bounce_collider.render(win, cam);
}

void Sludgelord::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	// debug();
}

fsm::StateFunction Sludgelord::update_idle() {
	p_state.actual = SludgelordState::idle;
	if (change_state(SludgelordState::turn, get_params("turn"))) { return SLUDGELORD_BIND(update_turn); }
	if (change_state(SludgelordState::roll, get_params("roll"))) { return SLUDGELORD_BIND(update_roll); }
	if (change_state(SludgelordState::bounce, get_params("bounce"))) { return SLUDGELORD_BIND(update_bounce); }
	if (change_state(SludgelordState::spit, get_params("spit"))) { return SLUDGELORD_BIND(update_spit); }
	return SLUDGELORD_BIND(update_idle);
}

fsm::StateFunction Sludgelord::update_spit() {
	p_state.actual = SludgelordState::spit;
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("sludgelord_burp", get_collider().get_center()); }
	if (p_animatable.frame_action(2) || p_animatable.frame_action(5)) { m_flags.set(SludgelordFlags::spit); }
	if (p_animatable.animation.is_complete()) {
		m_flags.reset(SludgelordFlags::bounced);
		request(SludgelordState::idle);
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
	}
	return SLUDGELORD_BIND(update_spit);
}

fsm::StateFunction Sludgelord::update_roll() {
	p_state.actual = SludgelordState::roll;
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("beast_growl", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		request(SludgelordState::slam);
		if (change_state(SludgelordState::slam, get_params("slam"))) { return SLUDGELORD_BIND(update_slam); }
	}
	return SLUDGELORD_BIND(update_roll);
}

fsm::StateFunction Sludgelord::update_slam() {
	p_state.actual = SludgelordState::slam;
	p_animatable.animation.linger_on_frame(0, !m_flags.test(SludgelordFlags::landed));
	if (get_collider().grounded() && !m_flags.test(SludgelordFlags::landed)) {
		m_flags.set(SludgelordFlags::landed);
		m_services->camera_controller.shake(5, 0.2f, 400, 30);
		p_services->soundboard.play_sound("juicy_impact", get_collider().get_center());
		p_services->soundboard.play_sound("vibration", get_collider().get_center());
		m_slam.start();
		p_map->spawn_effect(*p_services, "slam_impact", get_collider().get_center() + sf::Vector2f{0.f, 7.f});
	}
	if (p_animatable.animation.get_frame_count() < 3) { m_flags.set(SludgelordFlags::slamming); }
	if (p_animatable.animation.complete()) {
		m_flags.reset(SludgelordFlags::slamming);
		m_flags.reset(SludgelordFlags::landed);
		request(SludgelordState::idle);
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
	}
	return SLUDGELORD_BIND(update_slam);
}

fsm::StateFunction Sludgelord::update_bounce() {
	p_state.actual = SludgelordState::bounce;
	if (p_animatable.animation.just_started()) {
		m_bounce_collider.physics.velocity.y = -16.f;
		m_flags.set(SludgelordFlags::bounced);
	}
	if (m_bounce_collider.collided()) {
		m_bounce_counter.update();
		p_services->soundboard.play_sound("short_slam", get_collider().get_center());
		p_services->camera_controller.shake(4, 0.1f, 200, 20);
	}
	if (p_animatable.animation.complete() || m_bounce_collider.get_global_center().y > p_map->get_random_home_point().y || m_bounce_counter.get_count() > 2) {
		m_bounce_counter.start();
		half_health() ? request(SludgelordState::roar) : request(SludgelordState::idle);
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
		if (change_state(SludgelordState::roar, get_params("roar"))) { return SLUDGELORD_BIND(update_roar); }
	}
	return SLUDGELORD_BIND(update_bounce);
}

fsm::StateFunction Sludgelord::update_roar() {
	p_state.actual = SludgelordState::roar;
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("beast_growl", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		half_health() ? request(SludgelordState::spew) : request(SludgelordState::idle);
		if (change_state(SludgelordState::spew, get_params("spew"))) { return SLUDGELORD_BIND(update_spew); }
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
	}
	return SLUDGELORD_BIND(update_roar);
}

fsm::StateFunction Sludgelord::update_spew() {
	p_state.actual = SludgelordState::spew;
	if (p_services->ticker.every_x_ticks(15)) { m_spew.shoot(*p_services, *p_map); }
	// p_services->soundboard.repeat_sound("sludgelord_spew", 1U, get_collider().get_center());
	if (p_animatable.animation.complete()) {
		request(SludgelordState::idle);
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
	}
	return SLUDGELORD_BIND(update_spew);
}

fsm::StateFunction Sludgelord::update_turn() {
	p_state.actual = SludgelordState::turn;
	if (p_animatable.animation.complete()) {
		request_flip();
		request(SludgelordState::idle);
		if (change_state(SludgelordState::idle, get_params("idle"))) { return SLUDGELORD_BIND(update_idle); }
	}
	return SLUDGELORD_BIND(update_turn);
}

bool Sludgelord::change_state(SludgelordState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
