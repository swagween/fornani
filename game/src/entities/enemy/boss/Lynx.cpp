
#include <fornani/entities/enemy/boss/Lynx.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

static bool b_lynx_start{};
static bool b_lynx_debug{};
static void lynx_start_battle(int battle) { b_lynx_start = true; }
constexpr auto lynx_framerate = 10;
constexpr auto run_threshold_v = 0.002f;

Lynx::Lynx(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console)
	: Enemy(svc, "lynx"), NPC(svc, "lynx"), m_health_bar(svc, "lynx"), m_params{{"sit", {0, 1, lynx_framerate, -1}},
																				{"get_up", {1, 2, lynx_framerate * 4, 0}},
																				{"idle", {3, 6, lynx_framerate * 4, -1}},
																				{"jump", {9, 3, lynx_framerate * 2, 0}},
																				{"forward_slash", {47, 6, lynx_framerate * 2, 0}},
																				{"run", {14, 4, lynx_framerate * 2, -1}},
																				{"levitate", {18, 4, lynx_framerate * 4, 2}},
																				{"downward_slam", {22, 14, lynx_framerate * 3, 0}},
																				{"prepare_shuriken", {36, 3, lynx_framerate * 4, 0}},
																				{"toss_shuriken", {39, 6, lynx_framerate * 3, 2}},
																				{"triple_slash", {45, 18, lynx_framerate * 2, 0}},
																				{"upward_slash", {52, 5, lynx_framerate * 2, 0}},
																				{"turn", {56, 7, lynx_framerate * 2, 0}},
																				{"aerial_slash", {63, 4, lynx_framerate * 2, 0}},
																				{"prepare_slash", {45, 4, lynx_framerate * 4, 0}},
																				{"defeat", {0, 1, lynx_framerate, -1}}},
	  m_console{&console}, m_map{&map}, m_cooldowns{.run{240}, .post_hurt{64}}, m_services{&svc}, m_attacks{.left_shockwave{{30, 400, 2, {-1.5f, 0.f}}}, .right_shockwave{{30, 400, 2, {1.5f, 0.f}}}}, m_shuriken(svc, "shuriken") {
	Enemy::animation.set_params(get_params("sit"));
	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &lynx_start_battle));
	Enemy::collider.physics.set_friction_componentwise({0.97f, 0.99f});
	flags.state.set(StateFlags::no_shake);
	flags.general.set(GeneralFlags::post_death_render);
	flags.general.set(GeneralFlags::has_invincible_channel);
	state_flags.set(npc::NPCState::force_interact);
	flags.state.set(StateFlags::vulnerable);

	m_shuriken.get().set_team(arms::Team::skycorps);

	m_distant_range.set_dimensions({400, 800});

	m_home = {std::numeric_limits<float>::max(), 0.f};
	for (auto& pt : map.home_points) {
		m_home.x = std::min(pt.x, m_home.x);
		m_home.y = std::max(pt.x, m_home.y);
	}

	push_conversation(1);
}

void Lynx::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	Enemy::face_player(player);

	m_cooldowns.run.update();
	m_cooldowns.post_hurt.update();

	// positioning
	m_distant_range.set_position(Enemy::collider.bounding_box.get_position() - (m_distant_range.get_dimensions() * 0.5f) + (Enemy::collider.dimensions * 0.5f));
	m_player_target = player.collider.get_center() + sf::Vector2f{player.get_actual_direction().as_float() * 48.f, -180.f};
	if (is_state(LynxState::levitate)) {
		m_steering.seek(Enemy::collider.physics, m_player_target, 0.0005f);
		Enemy::collider.physics.simple_update();
	}

	// attacks

	// shuriken
	m_shuriken.update(svc, map, *this);
	auto bp = Enemy::collider.get_center();
	bp.x += 32.f * Enemy::directions.actual.as_float();
	m_shuriken.get().set_barrel_point(bp);
	m_attack_target = player.collider.get_center() + sf::Vector2f{0.f, -20.f} - m_shuriken.get().get_barrel_point();

	// melee
	if (m_flags.test(LynxFlags::battle_mode)) {
		if (m_attacks.left_shockwave.hit.active()) { player.hurt(); }
		if (m_attacks.right_shockwave.hit.active()) { player.hurt(); }
		for (auto& slash : m_attacks.slash) {
			auto damage = 1.f;
			slash.update();
			if (is_state(LynxState::downward_slam)) {
				slash.enable();
				if (Enemy::animation.get_frame_count() != 8 && Enemy::animation.get_frame_count() != 7) { slash.disable(); }
				damage = 2.f;
			}
			if (is_state(LynxState::upward_slash)) {
				slash.enable();
				if (Enemy::animation.get_frame_count() != 1) { slash.disable(); }
			}
			if (is_state(LynxState::aerial_slash)) {
				slash.enable();
				if (Enemy::animation.get_frame_count() != 1 && Enemy::animation.get_frame_count() != 2) { slash.disable(); }
			}
			if (is_state(LynxState::forward_slash)) {
				slash.enable();
				if (Enemy::animation.get_frame_count() != 2) { slash.disable(); }
			}
			if (is_state(LynxState::turn)) {
				slash.enable();
				if (Enemy::animation.get_frame_count() != 3) { slash.disable(); }
			}
			if (slash.hit.active()) {
				if (slash.hit.within_bounds(player.collider.bounding_box)) {
					if (!player.invincible()) { player.accumulated_forces.push_back({Enemy::directions.actual.as_float() * 4.f, -2.f}); }
					player.hurt(damage);
				}
				for (auto& proj : map.active_projectiles) {
					if (proj.get_team() == arms::Team::skycorps) { continue; }
					if (slash.hit.within_bounds(proj.get_bounding_box())) {
						map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
						random::percent_chance(50) ? svc.soundboard.flags.lynx.set(audio::Lynx::ping_1) : svc.soundboard.flags.lynx.set(audio::Lynx::ping_2);
						proj.destroy(false);
					}
				}
			}
		}
	}

	if (player.collider.get_center().y < Enemy::collider.vicinity.get_position().y && svc.ticker.every_second()) { random::percent_chance(50) ? request(LynxState::jump) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second()) { random::percent_chance(50) ? request(LynxState::run) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second() && half_health()) { random::percent_chance(50) ? request(LynxState::levitate) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second() /*&& half_health()*/ && random::percent_chance(50)) { request(LynxState::prepare_shuriken); }

	// caution
	auto incoming_projectile = m_caution.projectile_detected(map, Enemy::physical.alert_range, arms::Team::skycorps);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != Enemy::directions.actual.lnr) {
			random::percent_chance(50) ? request(LynxState::forward_slash) : request(LynxState::upward_slash);
			if (is_hurt()) { random::percent_chance(50) ? request(LynxState::jump) : request(LynxState::prepare_slash); }
		}
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start(128);
		if (!m_cooldowns.post_hurt.running()) {
			if (random::percent_chance(25)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_1);
			} else if (random::percent_chance(33)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_2);
			} else if (random::percent_chance(50)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_3);
			} else {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_3);
			}
		}
		m_cooldowns.post_hurt.start();
		flags.state.reset(StateFlags::hurt);
	}

	m_attacks.left_shockwave.origin = Enemy::collider.physics.position + sf::Vector2f{0.f, Enemy::collider.bounding_box.get_dimensions().y};
	m_attacks.right_shockwave.origin = Enemy::collider.physics.position + Enemy::collider.bounding_box.get_dimensions();
	m_attacks.left_shockwave.update(svc, map);
	m_attacks.right_shockwave.update(svc, map);

	m_attacks.left_shockwave.handle_player(player);
	m_attacks.right_shockwave.handle_player(player);

	m_health_bar.update(health.get_normalized());
	if (ccm::abs(Enemy::collider.physics.acceleration.x) > run_threshold_v) { request(LynxState::run); }
	if (Enemy::collider.get_center().x < m_home.x || Enemy::collider.get_center().x > m_home.y) { request(LynxState::jump); }
	if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { request(LynxState::turn); }

	// gameplay logic
	if (player.collider.bounding_box.overlaps(m_distant_range) && !state_flags.test(npc::NPCState::introduced) && state_flags.test(npc::NPCState::force_interact)) { triggers.set(npc::NPCTrigger::distant_interact); }
	NPC::update(svc, map, *m_console, player);
	if (b_lynx_start && !m_console->has_value()) {
		m_flags.set(LynxFlags::battle_mode);
		request(LynxState::get_up);
		b_lynx_start = false;
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
	}
	if (m_flags.test(LynxFlags::battle_mode)) { flags.state.set(StateFlags::vulnerable); }
	if (is_state(LynxState::downward_slam)) { flags.state.reset(StateFlags::vulnerable); }
	if (health.is_dead()) {
		post_death.start(afterlife);
		flags.state.set(StateFlags::special_death_mode);
		request(LynxState::defeat);
	}
	state_function = state_function();
}

void Lynx::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	NPC::render(svc, win, cam);
	Enemy::render(svc, win, cam);
	if (b_lynx_debug) {
		for (auto& slash : m_attacks.slash) {
			if (slash.hit.active()) { slash.render(win, cam); }
		}
		m_attacks.left_shockwave.render(win, cam);
		m_attacks.right_shockwave.render(win, cam);
	}
}

void Lynx::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (m_flags.test(LynxFlags::battle_mode)) { m_health_bar.render(win); }
	if (b_lynx_debug) { debug(); }
}

fsm::StateFunction Lynx::update_sit() {
	m_state.actual = LynxState::sit;
	if (change_state(LynxState::get_up, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::forward_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::triple_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::upward_slash, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	if (change_state(LynxState::downward_slam, get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	return LYNX_BIND(update_sit);
}

fsm::StateFunction Lynx::update_get_up() {
	m_state.actual = LynxState::get_up;
	m_flags.set(LynxFlags::battle_mode);
	if (Enemy::animation.complete()) {
		if (change_state(LynxState::aerial_slash, get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
		if (change_state(LynxState::forward_slash, get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
		if (change_state(LynxState::triple_slash, get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
		if (change_state(LynxState::upward_slash, get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
		if (change_state(LynxState::levitate, get_params("levitate"))) { return LYNX_BIND(update_levitate); }
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_get_up);
}

fsm::StateFunction Lynx::update_idle() {
	m_state.actual = LynxState::idle;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (change_state(LynxState::aerial_slash, get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
	if (change_state(LynxState::forward_slash, get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
	if (change_state(LynxState::triple_slash, get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
	if (change_state(LynxState::upward_slash, get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
	if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	if (change_state(LynxState::run, get_params("run"))) { return LYNX_BIND(update_run); }
	if (change_state(LynxState::jump, get_params("jump"))) { return LYNX_BIND(update_jump); }
	if (change_state(LynxState::levitate, get_params("levitate"))) { return LYNX_BIND(update_levitate); }
	if (change_state(LynxState::prepare_shuriken, get_params("prepare_shuriken"))) { return LYNX_BIND(update_prepare_shuriken); }
	if (change_state(LynxState::prepare_slash, get_params("prepare_slash"))) { return LYNX_BIND(update_prepare_slash); }
	return LYNX_BIND(update_idle);
}

fsm::StateFunction Lynx::update_jump() {
	m_state.actual = LynxState::jump;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }

	// check if battle is over and Lynx peaced out
	if (health.is_dead()) {
		flags.general.reset(GeneralFlags::map_collision);
		Enemy::collider.physics.acceleration.y = -80.f;
	} else {
		if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
			random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hah) : m_services->soundboard.flags.lynx.set(audio::Lynx::hoah);
			Enemy::collider.physics.acceleration.y = -200.f;
			Enemy::collider.physics.acceleration.x = Enemy::directions.actual.as_float() * 60.f;
		}
		if (Enemy::animation.complete()) {
			random::percent_chance(50) ? request(LynxState::aerial_slash) : request(LynxState::downward_slam);
			if (change_state(LynxState::aerial_slash, get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
			if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
		}
	}
	return LYNX_BIND(update_jump);
}

fsm::StateFunction Lynx::update_forward_slash() {
	m_state.actual = LynxState::forward_slash;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::heuh) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.get_frame_count() == 2 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::collider.get_center() + sf::Vector2f{36.f * sign * static_cast<float>(i), 0.f});
		slash.set_constant_radius(18.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_forward_slash);
}

fsm::StateFunction Lynx::update_levitate() {
	m_state.actual = LynxState::levitate;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	flags.general.reset(GeneralFlags::gravity);
	flags.state.set(StateFlags::simple_physics);
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		flags.state.reset(StateFlags::simple_physics);
		request(LynxState::downward_slam);
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_levitate);
}

fsm::StateFunction Lynx::update_run() {
	m_state.actual = LynxState::run;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	Enemy::collider.physics.acceleration.x = Enemy::directions.actual.as_float() * attributes.speed;
	if (Enemy::animation.just_started()) { m_cooldowns.run.start(); }
	if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (!m_cooldowns.run.running()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_run);
}

fsm::StateFunction Lynx::update_downward_slam() {
	m_state.actual = LynxState::downward_slam;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.get_frame_count() == 7) { Enemy::collider.physics.acceleration.y = 200.f; }
	if (Enemy::animation.get_frame_count() >= 7 && !Enemy::collider.grounded()) { Enemy::animation.set_frame(7); }
	if (Enemy::animation.get_frame_count() == 3 && Enemy::animation.keyframe_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::shing);
		m_services->soundboard.flags.lynx.set(audio::Lynx::hiyyah);
	}
	if (Enemy::animation.get_frame_count() == 8 && Enemy::animation.keyframe_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::slam);
		m_services->soundboard.flags.world.set(audio::World::thud);
	}
	Enemy::animation.get_frame_count() < 8 ? flags.general.reset(GeneralFlags::gravity) : flags.general.set(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto& slash : m_attacks.slash) {
		Enemy::animation.get_frame_count() == 8 ? slash.set_position(Enemy::collider.get_center() + sf::Vector2f{0.f, 30.f}) : slash.set_position(Enemy::collider.get_center() + sf::Vector2f{0.f, -20.f});
		slash.set_constant_radius(Enemy::animation.get_frame_count() == 7 ? 24.f : 64.f);
	}
	if (Enemy::animation.get_frame_count() == 9) {
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		m_attacks.left_shockwave.start();
		m_attacks.right_shockwave.start();
	}
	if (Enemy::animation.complete()) {
		flags.state.set(StateFlags::vulnerable);
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_downward_slam);
}

fsm::StateFunction Lynx::update_prepare_shuriken() {
	m_state.actual = LynxState::prepare_shuriken;
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::nngyah); }
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.complete()) {
		request(LynxState::toss_shuriken);
		if (change_state(LynxState::toss_shuriken, get_params("toss_shuriken"))) { return LYNX_BIND(update_toss_shuriken); }
	}
	return LYNX_BIND(update_prepare_shuriken);
}

fsm::StateFunction Lynx::update_toss_shuriken() {
	m_state.actual = LynxState::toss_shuriken;
	if (Enemy::animation.just_started() || ((Enemy::animation.get_frame_count() == 0 || Enemy::animation.get_frame_count() == 3) && Enemy::animation.keyframe_started())) {
		m_map->spawn_projectile_at(*m_services, m_shuriken.get(), m_shuriken.get().get_barrel_point(), m_attack_target);
		m_services->soundboard.flags.lynx.set(audio::Lynx::shing);
	}
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_toss_shuriken);
}

fsm::StateFunction Lynx::update_upward_slash() {
	m_state.actual = LynxState::upward_slash;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hoah) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto& slash : m_attacks.slash) {
		slash.set_position(Enemy::collider.get_center() + sf::Vector2f{36.f * sign, -20.f});
		slash.set_constant_radius(64.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_upward_slash);
}

fsm::StateFunction Lynx::update_triple_slash() {
	m_state.actual = LynxState::triple_slash;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	auto sign = Enemy::directions.actual.as_float();
	for (auto& slash : m_attacks.slash) {
		slash.set_position(Enemy::collider.get_center() + sf::Vector2f{36.f * sign, -20.f});
		slash.set_constant_radius(64.f);
	}
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_triple_slash);
}

fsm::StateFunction Lynx::update_turn() {
	m_state.actual = LynxState::turn;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::huuyeah); }
	Enemy::directions.desired.lock();
	if (Enemy::animation.get_frame_count() == 3 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::collider.get_center() + sf::Vector2f{-28.f * sign * static_cast<float>(i), static_cast<float>(i) * -8.f});
		if (i == 0) { slash.set_position(Enemy::collider.get_center() + sf::Vector2f{16.f * sign, 8.f}); }
		slash.set_constant_radius(30.f);
		if (i == 1) { slash.set_constant_radius(40.f); }
	}
	if (Enemy::animation.complete()) {
		request_flip();
		request(LynxState::idle);
		if (change_state(LynxState::idle, get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_turn);
}

fsm::StateFunction Lynx::update_aerial_slash() {
	m_state.actual = LynxState::aerial_slash;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hoah) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.just_started()) { Enemy::collider.physics.acceleration.x = Enemy::directions.actual.as_float() * 600.f; }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::collider.get_center() + sf::Vector2f{36.f * sign * static_cast<float>(i), 0.f});
		i == 1 ? slash.set_constant_radius(22.f) : slash.set_constant_radius(18.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		if (Enemy::collider.grounded()) {
			if (change_state(LynxState::turn, get_params("turn"))) { return LYNX_BIND(update_turn); }
		}
		Enemy::collider.grounded() ? request(LynxState::upward_slash) : request(LynxState::downward_slam);
		if (change_state(LynxState::upward_slash, get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
		if (change_state(LynxState::downward_slam, get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_aerial_slash);
}

fsm::StateFunction Lynx::update_prepare_slash() {
	m_state.actual = LynxState::prepare_slash;
	if (change_state(LynxState::defeat, get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::hnnyah); }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::prepare); }
	if (Enemy::animation.complete()) {
		request(LynxState::aerial_slash);
		if (change_state(LynxState::aerial_slash, get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
	}
	return LYNX_BIND(update_prepare_slash);
}

fsm::StateFunction Lynx::update_defeat() {
	m_state.actual = LynxState::defeat;
	if (Enemy::animation.just_started()) {
		m_flags.reset(LynxFlags::battle_mode);
		triggers.set(npc::NPCTrigger::distant_interact);
		flush_conversations();
		push_conversation(2);
		m_services->music_player.pause();
		m_services->soundboard.flags.lynx.set(audio::Lynx::defeat);
	}
	if (Enemy::animation.complete()) {
		request(LynxState::jump);
		if (change_state(LynxState::jump, get_params("jump"))) { return LYNX_BIND(update_jump); }
	}
	return LYNX_BIND(update_defeat);
}

bool Lynx::change_state(LynxState next, anim::Parameters params) {
	if (m_state.desired == next) {
		Enemy::animation.set_params(params);
		return true;
	}
	return false;
}

anim::Parameters const& Lynx::get_params(std::string const& key) { return m_params.contains(key) ? m_params.at(key) : m_params.at("idle"); }

void Lynx::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Lynx Controls")) {
		if (ImGui::Button("aerial_slash")) { request(LynxState::aerial_slash); }
		if (ImGui::Button("forward_slash")) { request(LynxState::forward_slash); }
		if (ImGui::Button("upward_slash")) { request(LynxState::upward_slash); }
		if (ImGui::Button("triple_slash")) { request(LynxState::triple_slash); }
		if (ImGui::Button("downward_slam")) { request(LynxState::downward_slam); }
		if (ImGui::Button("run")) { request(LynxState::run); }
		if (ImGui::Button("jump")) { request(LynxState::jump); }
		if (ImGui::Button("levitate")) { request(LynxState::levitate); }
		if (ImGui::Button("toss_shuriken")) { request(LynxState::prepare_shuriken); }
		if (ImGui::Button("slash")) { request(LynxState::prepare_slash); }
		ImGui::End();
	}
}

} // namespace fornani::enemy
