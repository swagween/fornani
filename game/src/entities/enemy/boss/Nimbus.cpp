
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include <fornani/entities/enemy/boss/Nimbus.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Nimbus::Nimbus(automa::ServiceProvider& svc, world::Map& map) : Boss(svc, map, "nimbus"), m_slash_wave(svc, "slash_wave"), m_map{&map}, m_services{&svc} {
	auto fr = 5;
	p_animatable.set_animations({{"idle", {93, 4, fr * 8, -1}},
								 {"jump", {1, 7, fr * 5, 0, true}},
								 {"land", {8, 7, fr * 3, 0}},
								 {"get_up", {12, 2, fr * 4, 0}},
								 {"prepare_forward_slash", {16, 3, fr * 5, 0}},
								 {"forward_slash", {19, 9, fr * 2, 0}},
								 {"prepare_twirl_cut", {29, 4, fr * 6, 0}},
								 {"twirl_cut", {33, 6, fr * 2, 0}},
								 {"flourish", {39, 9, fr * 3, 0}},
								 {"downward_cut", {48, 8, fr * 3, 0}},
								 {"upward_cut", {56, 6, fr * 3, 0}},
								 {"turn", {62, 7, fr * 2, 0}},
								 {"parry", {69, 3, fr * 4, 0}},
								 {"begin_levitate", {72, 5, fr * 2, 0}},
								 {"levitate", {77, 4, fr * 4, -1}},
								 {"spellcast", {81, 3, fr * 2, 3}},
								 {"double_aerial_slash", {84, 9, fr * 2, 0}},
								 {"frenzy_prepare", {97, 5, fr * 5, 0}},
								 {"frenzy_quick_prepare", {99, 3, fr * 4, 0}},
								 {"frenzy_dash", {102, 4, fr * 3, 0}},
								 {"frenzy_cuts", {106, 19, fr * 2, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	get_collider().physics.ground_friction = {0.95f, 0.999f};
	get_collider().physics.air_friction = {0.99f, 0.999f};
	m_slash_wave.get().set_team(arms::Team::skycorps);
	flags.general.set(GeneralFlags::has_invincible_channel);
	flags.state.set(StateFlags::vulnerable);
	start_battle();
}

void Nimbus::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	if (consume_flag(BossFlags::start_battle)) {
		svc.music_player.load(svc.finder, "tumult");
		svc.music_player.play_looped();
	}

	// logic
	m_cooldowns.post_slash.update();
	m_cooldowns.parry.update();
	m_cooldowns.post_jump.update();
	if (is_airborne()) { m_steering.thrust_seek(get_collider().physics, m_steer_target, {0.03f, .118f, .991f, 260.f}); }
	m_slash_wave.update(svc, map, *this);
	auto bp = Enemy::get_collider().get_center();
	m_slash_wave.get().set_barrel_point(bp);

	// melee attacks
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
		auto const frenzy_position = Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 40.f, 8.f};
		auto const frame = p_animatable.animation.get_frame_count();
		m_attacks.frenzy.set_position(frenzy_position);
		m_attacks.frenzy.set_constant_radius(24.f);
		if (is_state(NimbusState::frenzy_dash) && m_attacks.frenzy.sensor.is_very_near(player.hurtbox) && m_attacks.frenzy.sensor.active() && !m_flags.test(NimbusFlags::frenzy_contact)) {
			m_flags.set(NimbusFlags::frenzy_contact);
			player.stun(0.5f);
			auto const freeze = 0.1f;
			svc.ticker.freeze_frame(freeze);
		}
		m_attacks.frenzy.disable();
		if (is_state(NimbusState::frenzy_cuts)) {
			m_attacks.frenzy.enable();
			if (frame < 19) { player.set_position(frenzy_position, true); }
			auto const active = frame == 7 || frame == 10 || frame == 15;
			if (!active) { m_attacks.frenzy.disable(); }
			if (p_animatable.animation.keyframe_started()) {
				if (active) {
					m_attacks.frenzy.hurt_player(player, 1.f, {}, true);
					auto const pos = frenzy_position;
					svc.soundboard.play_sound("nimbus_clang", pos);
					random::coin_flip() ? svc.soundboard.play_sound("juicy_impact", pos) : svc.soundboard.play_sound("clean_impact", pos);
					map.spawn_frame_effect(svc, "triangle_flash", pos, {}, 1);
				}
			}
		}

		for (auto& slash : m_attacks.slash) {
			auto damage = 1.f;
			slash.disable();
			slash.update();
			if (is_state(NimbusState::upward_cut)) {
				slash.enable();
				if (frame != 1) { slash.disable(); }
				damage = 1.f;
			}
			if (is_state(NimbusState::turn)) {
				slash.enable();
				if (frame != 3 && frame != 4) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(NimbusState::forward_slash)) {
				slash.enable();
				if (frame != 1 && frame != 2) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(NimbusState::downward_cut)) {
				slash.enable();
				if (frame != 3 && frame != 4) { slash.disable(); }
				damage = 2.f;
			}
			slash.hurt_player(player, damage, {directions.desired.as_float() * 0.4f, -0.2f});
			slash.cancel_projectiles(svc, map, get_team());
		}
	}

	auto const manual = false;
	if (!manual) {
		if (svc.ticker.every_second()) {
			if (!m_cooldowns.post_slash.running()) {
				random::coin_flip() ? request(NimbusState::frenzy_prepare) : request(NimbusState::prepare_twirl_cut);
				// random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut);
				if (random::percent_chance(35)) { request(NimbusState::flourish); }
			} else {
			}
		}
	}

	// animation
	if (m_caution.is_projectile_detected(map, physical.alert_range, arms::Team::skycorps) && !m_cooldowns.parry.running()) {
		request(NimbusState::parry);
	} else if (is_state(NimbusState::parry)) {
		request(NimbusState::idle);
	}
	if (!manual) {
		if (is_hostile()) { request(NimbusState::upward_cut); }
	}
	if (directions.actual.lnr != directions.desired.lnr && !is_airborne()) { request(NimbusState::turn); }
	if (player.is_dead()) { request(NimbusState::idle); }

	state_function = state_function();
}

void Nimbus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	if (!debug::is_production()) {
		for (auto& slash : m_attacks.slash) {
			if (slash.hit.active()) { slash.render(win, cam); }
		}
	}
	// m_attacks.frenzy.render(win, cam);
	debug();
}

void Nimbus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Boss::gui_render(svc, win, cam); }

fsm::StateFunction Nimbus::update_idle() {
	p_state.actual = NimbusState::idle;
	if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
	if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
	if (change_state(NimbusState::frenzy_prepare, Enemy::get_params("frenzy_prepare"))) { return NIMBUS_BIND(update_frenzy_prepare); }
	if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
	if (change_state(NimbusState::flourish, Enemy::get_params("flourish"))) { return NIMBUS_BIND(update_flourish); }
	if (change_state(NimbusState::downward_cut, Enemy::get_params("downward_cut"))) { return NIMBUS_BIND(update_downward_cut); }
	if (change_state(NimbusState::upward_cut, Enemy::get_params("upward_cut"))) { return NIMBUS_BIND(update_upward_cut); }
	if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
	if (change_state(NimbusState::parry, Enemy::get_params("parry"))) { return NIMBUS_BIND(update_parry); }
	if (change_state(NimbusState::begin_levitate, Enemy::get_params("begin_levitate"))) { return NIMBUS_BIND(update_begin_levitate); }
	return NIMBUS_BIND(update_idle);
}

fsm::StateFunction Nimbus::update_jump() {
	p_state.actual = NimbusState::jump;
	if (p_animatable.animation.just_started()) { m_triple_jump.update(); }
	auto const force = m_flags.test(NimbusFlags::frenzy_over) ? -4.f : -15.f;
	auto const x_force = m_flags.test(NimbusFlags::frenzy_over) ? -directions.actual.as_float() * 0.2f : directions.actual.as_float();
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) { get_collider().physics.velocity = sf::Vector2f{x_force * 5.f, force}; }
	if (p_animatable.animation.get_frame_count() >= 1) { get_collider().physics.apply_force({x_force, 0.f}); }
	if (p_animatable.animation.get_frame_count() == 6 && is_alert() && !m_flags.test(NimbusFlags::frenzy_over)) { request(NimbusState::downward_cut); }
	if (change_state(NimbusState::downward_cut, get_params("downward_cut"))) { return NIMBUS_BIND(update_downward_cut); }
	if (p_animatable.animation.is_complete() && get_collider().grounded()) {
		m_flags.reset(NimbusFlags::frenzy_over);
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_jump);
}

fsm::StateFunction Nimbus::update_land() {
	p_state.actual = NimbusState::land;
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_land);
}

fsm::StateFunction Nimbus::update_get_up() {
	p_state.actual = NimbusState::get_up;
	if (p_animatable.animation.is_complete()) {
		if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
		if (m_triple_jump.get_count() < 3 && !m_cooldowns.post_jump.running()) {
			request(NimbusState::jump);
		} else {
			if (m_triple_jump.get_count() >= 3) { m_cooldowns.post_jump.start(); }
			m_triple_jump.start();
		}
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
		if (!m_cooldowns.post_slash.running()) { random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
		if (random::percent_chance(25) && !m_cooldowns.post_jump.running()) { request(NimbusState::jump); }
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_get_up);
}

fsm::StateFunction Nimbus::update_prepare_forward_slash() {
	p_state.actual = NimbusState::prepare_forward_slash;
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::forward_slash);
		if (change_state(NimbusState::forward_slash, Enemy::get_params("forward_slash"))) { return NIMBUS_BIND(update_forward_slash); }
	}
	return NIMBUS_BIND(update_prepare_forward_slash);
}

fsm::StateFunction Nimbus::update_forward_slash() {
	p_state.actual = NimbusState::forward_slash;
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_1", get_collider().get_center()); }
	if (p_animatable.animation.just_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 40.f, -7.f}; }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 28.f, 8.f});
		i == 1 ? slash.set_constant_radius(40.f) : slash.set_constant_radius(18.f);
	}
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		m_cooldowns.post_slash.start();
		if (change_state(NimbusState::turn, Enemy::get_params("turn"))) { return NIMBUS_BIND(update_turn); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_forward_slash);
}

fsm::StateFunction Nimbus::update_prepare_twirl_cut() {
	p_state.actual = NimbusState::prepare_twirl_cut;
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::twirl_cut);
		if (change_state(NimbusState::twirl_cut, Enemy::get_params("twirl_cut"))) { return NIMBUS_BIND(update_twirl_cut); }
	}
	return NIMBUS_BIND(update_prepare_twirl_cut);
}

fsm::StateFunction Nimbus::update_twirl_cut() {
	p_state.actual = NimbusState::twirl_cut;
	if (p_animatable.animation.just_started()) {
		m_slash_wave.shoot(*m_services, *m_map, (get_collider().get_center() + sf::Vector2f{directions.actual.as_float(), -1.f}) - get_collider().get_center());
		m_services->soundboard.play_sound("nimbus_swipe_2", get_collider().get_center());
	}
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_twirl_cut);
}

fsm::StateFunction Nimbus::update_flourish() {
	p_state.actual = NimbusState::flourish;
	if (p_animatable.animation.is_complete()) {
		if (!m_cooldowns.post_slash.running()) { request(NimbusState::prepare_forward_slash); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_flourish);
}

fsm::StateFunction Nimbus::update_downward_cut() {
	p_state.actual = NimbusState::downward_cut;
	if (p_animatable.animation.get_frame_count() == 3 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_1", get_collider().get_center()); }
	if (p_animatable.animation.just_started()) {
		if (directions.desired != directions.actual) { request_flip(); }
	}
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 28.f, 10.f});
		i == 1 ? slash.set_constant_radius(32.f) : slash.set_constant_radius(18.f);
	}
	if (p_animatable.animation.get_frame_count() < 2) { get_collider().physics.zero(); }
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 28.f, 18.f}; }
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::get_up);
		if (change_state(NimbusState::get_up, Enemy::get_params("get_up"))) { return NIMBUS_BIND(update_get_up); }
	}
	return NIMBUS_BIND(update_downward_cut);
}

fsm::StateFunction Nimbus::update_upward_cut() {
	p_state.actual = NimbusState::upward_cut;
	if (p_animatable.animation.get_frame_count() == 1 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_3", get_collider().get_center()); }
	if (p_animatable.animation.just_started()) { m_slash_wave.shoot(*m_services, *m_map, (get_collider().get_center() + sf::Vector2f{0.f, -1.f}) - get_collider().get_center()); }
	auto sign = directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{0.f, -18.f});
		i == 1 ? slash.set_constant_radius(40.f) : slash.set_constant_radius(18.f);
	}
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_upward_cut);
}

fsm::StateFunction Nimbus::update_turn() {
	p_state.actual = NimbusState::turn;
	directions.desired.lock();
	if (p_animatable.animation.get_frame_count() == 3 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("nimbus_swipe_3", get_collider().get_center()); }
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * -20.f, -18.f});
		i == 1 ? slash.set_constant_radius(44.f) : slash.set_constant_radius(18.f);
	}
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		m_cooldowns.post_slash.start();
		request_flip();
		request(NimbusState::get_up);
		if (change_state(NimbusState::get_up, Enemy::get_params("get_up"))) { return NIMBUS_BIND(update_get_up); }
	}
	return NIMBUS_BIND(update_turn);
}

fsm::StateFunction Nimbus::update_parry() {
	p_state.actual = NimbusState::parry;
	p_animatable.animation.get_frame_count() == 0 ? flags.state.reset(StateFlags::vulnerable) : flags.state.set(StateFlags::vulnerable);
	if (change_state(NimbusState::upward_cut, Enemy::get_params("upward_cut"))) { return NIMBUS_BIND(update_upward_cut); }
	if (flags.state.consume(StateFlags::blocked_projectile)) {
		m_services->soundboard.play_sound("nimbus_parry", get_collider().get_center());
		m_services->ticker.freeze_frame(0.05f);
		flags.state.reset(StateFlags::vulnerable);
		m_cooldowns.parry.start();
		random::percent_chance(50) ? request(NimbusState::frenzy_quick_prepare) : request(NimbusState::frenzy_quick_prepare);
		if (change_state(NimbusState::frenzy_quick_prepare, Enemy::get_params("frenzy_quick_prepare"))) { return NIMBUS_BIND(update_frenzy_quick_prepare); }
		if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
	}
	if (change_state(NimbusState::parry, Enemy::get_params("parry"))) { return NIMBUS_BIND(update_parry); }
	if (p_animatable.animation.is_complete()) {
		m_cooldowns.parry.start(64);
		flags.state.set(StateFlags::vulnerable);
		if (random::percent_chance(25) && !m_cooldowns.post_jump.running()) { request(NimbusState::jump); }
		if (!m_cooldowns.post_slash.running()) { random::coin_flip() ? request(NimbusState::prepare_forward_slash) : request(NimbusState::prepare_twirl_cut); }
		if (change_state(NimbusState::prepare_forward_slash, Enemy::get_params("prepare_forward_slash"))) { return NIMBUS_BIND(update_prepare_forward_slash); }
		if (change_state(NimbusState::prepare_twirl_cut, Enemy::get_params("prepare_twirl_cut"))) { return NIMBUS_BIND(update_prepare_twirl_cut); }
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
		request(NimbusState::idle);
		if (change_state(NimbusState::idle, Enemy::get_params("idle"))) { return NIMBUS_BIND(update_idle); }
	}
	return NIMBUS_BIND(update_parry);
}

fsm::StateFunction Nimbus::update_begin_levitate() {
	p_state.actual = NimbusState::begin_levitate;
	get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().set_flag(shape::ColliderFlags::gravity, false);
	if (p_animatable.animation.just_started()) { m_steer_target = m_map->get_random_home_point(); }
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::levitate);
		if (change_state(NimbusState::levitate, Enemy::get_params("levitate"))) { return NIMBUS_BIND(update_levitate); }
	}
	return NIMBUS_BIND(update_begin_levitate);
}

fsm::StateFunction Nimbus::update_levitate() {
	p_state.actual = NimbusState::levitate;
	if (change_state(NimbusState::spellcast, Enemy::get_params("spellcast"))) { return NIMBUS_BIND(update_spellcast); }
	return NIMBUS_BIND(update_levitate);
}

fsm::StateFunction Nimbus::update_spellcast() {
	p_state.actual = NimbusState::spellcast;
	if (change_state(NimbusState::double_aerial_slash, Enemy::get_params("double_aerial_slash"))) { return NIMBUS_BIND(update_double_aerial_slash); }
	return NIMBUS_BIND(update_spellcast);
}

fsm::StateFunction Nimbus::update_double_aerial_slash() {
	p_state.actual = NimbusState::double_aerial_slash;
	get_collider().set_flag(shape::ColliderFlags::simple, false);
	get_collider().set_flag(shape::ColliderFlags::gravity);
	if (p_animatable.animation.get_frame_count() < 2) { get_collider().physics.zero(); }
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_double_aerial_slash);
}

fsm::StateFunction Nimbus::update_frenzy_prepare() {
	p_state.actual = NimbusState::frenzy_prepare;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("nimbus_shing", get_collider().get_center()); }
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::frenzy_dash);
		if (change_state(NimbusState::frenzy_dash, Enemy::get_params("frenzy_dash"))) { return NIMBUS_BIND(update_frenzy_dash); }
	}
	return NIMBUS_BIND(update_frenzy_prepare);
}

fsm::StateFunction Nimbus::update_frenzy_quick_prepare() {
	p_state.actual = NimbusState::frenzy_quick_prepare;
	if (p_animatable.animation.just_started()) { m_services->soundboard.play_sound("nimbus_shing", get_collider().get_center()); }
	if (p_animatable.animation.is_complete()) {
		request(NimbusState::frenzy_dash);
		if (change_state(NimbusState::frenzy_dash, Enemy::get_params("frenzy_dash"))) { return NIMBUS_BIND(update_frenzy_dash); }
	}
	return NIMBUS_BIND(update_frenzy_quick_prepare);
}

fsm::StateFunction Nimbus::update_frenzy_dash() {
	p_state.actual = NimbusState::frenzy_dash;
	if (p_animatable.animation.just_started()) { /*m_services->soundboard.play_sound("nimbus_dash", get_collider().get_center());*/
	}
	get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 52.f, 0.f};
	m_attacks.frenzy.sensor.activate();
	if (m_flags.consume(NimbusFlags::frenzy_contact)) {
		get_collider().physics.zero();
		m_attacks.frenzy.sensor.deactivate();
		request(NimbusState::frenzy_cuts);
		if (change_state(NimbusState::frenzy_cuts, Enemy::get_params("frenzy_cuts"))) { return NIMBUS_BIND(update_frenzy_cuts); }
	}
	if (p_animatable.animation.is_complete()) {
		m_attacks.frenzy.sensor.deactivate();
		flags.state.set(StateFlags::vulnerable);
		m_cooldowns.post_slash.start();
		request(NimbusState::land);
		if (change_state(NimbusState::land, Enemy::get_params("land"))) { return NIMBUS_BIND(update_land); }
	}
	return NIMBUS_BIND(update_frenzy_dash);
}

fsm::StateFunction Nimbus::update_frenzy_cuts() {
	p_state.actual = NimbusState::frenzy_cuts;
	if (p_animatable.animation.just_started()) {
		get_collider().physics.velocity = sf::Vector2f{directions.actual.as_float() * 3.f, -18.f};
		m_services->soundboard.play_sound("nimbus_frenzy_kick", get_collider().get_center());
	}
	if (p_animatable.animation.get_frame_count() >= 5) { get_collider().physics.zero_y(); }
	if (p_animatable.animation.is_complete()) {
		flags.state.set(StateFlags::vulnerable);
		m_flags.set(NimbusFlags::frenzy_over);
		request(NimbusState::jump);
		if (change_state(NimbusState::jump, Enemy::get_params("jump"))) { return NIMBUS_BIND(update_jump); }
	}
	return NIMBUS_BIND(update_frenzy_cuts);
}

void Nimbus::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Nimbus Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Post Jump: ");
		ImGui::ProgressBar(m_cooldowns.post_jump.get_normalized());
		ImGui::Text("Parry: ");
		ImGui::ProgressBar(m_cooldowns.parry.get_normalized());
		ImGui::Text("Jump Counter: %i", m_triple_jump.get_count());
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("jump")) { request(NimbusState::jump); }
		if (ImGui::Button("land")) { request(NimbusState::land); }
		if (ImGui::Button("frenzy_prepare")) { request(NimbusState::frenzy_prepare); }
		if (ImGui::Button("prepare_forward_slash")) { request(NimbusState::prepare_forward_slash); }
		if (ImGui::Button("prepare_twirl_cut")) { request(NimbusState::prepare_twirl_cut); }
		if (ImGui::Button("flourish")) { request(NimbusState::flourish); }
		if (ImGui::Button("downward_cut")) { request(NimbusState::downward_cut); }
		if (ImGui::Button("upward_cut")) { request(NimbusState::upward_cut); }
		if (ImGui::Button("turn")) { request(NimbusState::turn); }
		if (ImGui::Button("parry")) { request(NimbusState::parry); }
		if (ImGui::Button("begin_levitate")) { request(NimbusState::begin_levitate); }
		if (ImGui::Button("spellcast")) { request(NimbusState::spellcast); }
		if (ImGui::Button("double_aerial_slash")) { request(NimbusState::double_aerial_slash); }
		ImGui::End();
	}
}

bool Nimbus::change_state(NimbusState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
