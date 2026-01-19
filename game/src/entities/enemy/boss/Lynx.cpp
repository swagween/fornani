
#include <fornani/entities/enemy/boss/Lynx.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

static bool b_lynx_start{};
static bool b_lynx_debug{};
static void lynx_start_battle(int battle) { b_lynx_start = true; }
constexpr auto lynx_framerate = 7;
constexpr auto run_threshold_v = 0.002f;

Lynx::Lynx(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console)
	: NPC(svc, map, std::string_view{"lynx"}, false), Boss(svc, map, "lynx"), m_console{&console}, m_map{&map},
	  m_cooldowns{.run{240}, .post_hurt{64}, .post_shuriken_toss{1200}, .post_levitate{1000}, .start_levitate{150}, .throw_shuriken{60}, .post_defeat{800}}, m_services{&svc},
	  m_attacks{.left_shockwave{{30, 400, 2, {-1.5f, 0.f}}}, .right_shockwave{{30, 400, 2, {1.5f, 0.f}}}}, m_shuriken(svc, "shuriken"), m_magic{svc, {40.f, 40.f}, colors::white, "lynx_magic"}, m_seek_friction{0.9f, 0.9f} {
	Enemy::m_params = {
		{"sit", {0, 1, lynx_framerate, -1}},
		{"get_up", {1, 2, lynx_framerate * 4, 0}},
		{"idle", {3, 6, lynx_framerate * 4, -1}},
		{"jump", {9, 5, lynx_framerate * 3, 0}},
		{"forward_slash", {47, 6, lynx_framerate * 3, 0}},
		{"run", {14, 4, lynx_framerate * 2, -1}},
		{"levitate", {18, 4, lynx_framerate * 4, 2}},
		{"downward_slam", {22, 14, lynx_framerate * 3, 0}},
		{"prepare_shuriken", {36, 3, lynx_framerate * 6, 0}},
		{"toss_shuriken", {39, 6, lynx_framerate * 3, 1}},
		{"triple_slash", {45, 18, lynx_framerate * 2, 0}},
		{"upward_slash", {52, 5, lynx_framerate * 3, 0}},
		{"turn", {55, 8, lynx_framerate * 2, 0}},
		{"aerial_slash", {63, 4, lynx_framerate * 2, 0}},
		{"prepare_slash", {45, 4, lynx_framerate * 5, 0}},
		{"defeat", {67, 6, lynx_framerate * 6, -1}},
		{"second_phase", {18, 4, lynx_framerate * 4, -1}},
		{"laugh", {73, 4, lynx_framerate * 3, 4}},
		{"stagger", {77, 1, lynx_framerate * 4, -1}},
	};
	Enemy::animation.set_params(Enemy::get_params("sit"));
	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &lynx_start_battle));
	flags.state.set(StateFlags::no_shake);
	flags.general.set(GeneralFlags::post_death_render);
	set_force_interact(true);
	flags.state.reset(StateFlags::vulnerable);

	m_shuriken.get().set_team(arms::Team::skycorps);
	m_magic.deactivate();
	m_distant_range.set_dimensions({450, 800});

	m_home = {std::numeric_limits<float>::max(), 0.f};
	for (auto& pt : map.home_points) {
		m_home.x = std::min(pt.x, m_home.x);
		m_home.y = std::max(pt.x, m_home.y);
	}
	flags.state.set(StateFlags::no_slowdown);

	auto prog = svc.quest_table.get_quest_progression("lynx_dialogue");
	auto which = prog == 0 ? 1 : 4;
	push_conversation(which);
}

void Lynx::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	m_cooldowns.run.update();
	m_cooldowns.post_hurt.update();
	m_cooldowns.start_levitate.update();
	m_cooldowns.post_shuriken_toss.update();
	m_cooldowns.post_levitate.update();
	m_cooldowns.throw_shuriken.update();
	if (!m_console->has_value()) { m_cooldowns.post_defeat.update(); }

	Enemy::get_collider().has_flag_set(shape::ColliderFlags::simple) ? Enemy::get_collider().physics.set_friction_componentwise(m_seek_friction) : Enemy::get_collider().physics.set_friction_componentwise({0.97f, 0.99f});

	Enemy::update(svc, map, player);
	Enemy::face_player(player);

	// positioning
	m_distant_range.set_position(Enemy::get_collider().bounding_box.get_position() - (m_distant_range.get_dimensions() * 0.5f) + (Enemy::get_collider().dimensions * 0.5f));
	m_player_target = player.get_collider().get_center() + sf::Vector2f{player.get_actual_direction().as_float() * 50.f, -200.f};

	if (Enemy::get_collider().has_flag_set(shape::ColliderFlags::simple)) {
		if (is_levitating()) { m_steering.seek(Enemy::get_collider().physics, m_player_target, 0.00045f); }
		if (slam_follow()) { m_steering.seek(Enemy::get_collider().physics, m_player_target, 0.0004f); }
	}

	// effects
	m_magic.set_position(Enemy::get_collider().physics.position);
	m_magic.update(svc);

	// attacks
	// shuriken
	m_shuriken.update(svc, map, *this);
	auto bp = Enemy::get_collider().get_center();
	if (!is_levitating()) { bp.x += 32.f * Enemy::directions.actual.as_float(); }
	m_shuriken.get().set_barrel_point(bp);
	m_attack_target = player.get_collider().get_center() + sf::Vector2f{0.f, -20.f} - m_shuriken.get().get_barrel_point();

	// melee
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
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
				if (Enemy::animation.get_frame_count() != 4) { slash.disable(); }
			}
			if (slash.hit.active()) {
				if (slash.hit.within_bounds(player.get_collider().bounding_box)) {
					if (!player.invincible()) { player.accumulated_forces.push_back({Enemy::directions.actual.as_float() * 4.f, -2.f}); }
					player.hurt(damage);
				}
				for (auto& proj : map.active_projectiles) {
					if (proj.get_team() == arms::Team::skycorps) { continue; }
					if (slash.hit.within_bounds(proj.get_collider())) {
						map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
						random::percent_chance(50) ? svc.soundboard.flags.lynx.set(audio::Lynx::ping_1) : svc.soundboard.flags.lynx.set(audio::Lynx::ping_2);
						proj.destroy(false);
						svc.ticker.freeze_frame(6);
					}
				}
			}
		}
	}

	if (player.get_collider().get_center().y < Enemy::get_collider().get_vicinity_rect().position.y && svc.ticker.every_second()) { random::percent_chance(50) ? request(LynxState::jump) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second()) { random::percent_chance(60) ? request(LynxState::run) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second() && half_health()) { random::percent_chance(80) && !m_cooldowns.post_levitate.running() ? request(LynxState::levitate) : request(LynxState::prepare_slash); }
	if (is_state(LynxState::idle) && svc.ticker.every_second() && half_health() && random::percent_chance(20) && !m_cooldowns.post_shuriken_toss.running()) { request(LynxState::prepare_shuriken); }
	if (is_state(LynxState::idle) && svc.ticker.every_second() && quarter_health() && random::percent_chance(70) && !m_cooldowns.post_levitate.running()) { request(LynxState::levitate); }

	// caution
	if (Boss::has_flag_set(BossFlags::battle_mode)) {
		auto incoming_projectile = m_caution.projectile_detected(map, Enemy::physical.alert_range, arms::Team::skycorps);
		if (incoming_projectile.lnr != LNR::neutral) {
			if (incoming_projectile.lnr != Enemy::directions.actual.lnr && random::percent_chance(90)) {
				random::percent_chance(50) ? request(LynxState::forward_slash) : request(LynxState::upward_slash);
				if (is_hurt()) {
					random::percent_chance(50) ? request(LynxState::jump) : request(LynxState::prepare_slash);
					if (random::percent_chance(15)) { request(LynxState::run); }
					if (half_health()) {
						if (random::percent_chance(50) && !m_cooldowns.post_shuriken_toss.running()) { request(LynxState::prepare_shuriken); }
					}
					if (quarter_health()) {
						if (random::percent_chance(50) && !m_cooldowns.post_levitate.running()) { request(LynxState::levitate); }
					}
				}
			}
		}
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		if (!m_cooldowns.post_hurt.running()) {
			if (random::percent_chance(25)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_1);
			} else if (random::percent_chance(33)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_2);
			} else if (random::percent_chance(50)) {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_3);
			} else {
				m_services->soundboard.flags.lynx.set(audio::Lynx::hurt_4);
			}
		}
		m_cooldowns.post_hurt.start();
		flags.state.reset(StateFlags::hurt);
	}

	// shockwaves
	m_attacks.left_shockwave.origin = Enemy::get_collider().physics.position + sf::Vector2f{0.f, Enemy::get_collider().bounding_box.get_dimensions().y};
	m_attacks.right_shockwave.origin = Enemy::get_collider().physics.position + Enemy::get_collider().bounding_box.get_dimensions();
	m_attacks.left_shockwave.update(svc, map);
	m_attacks.right_shockwave.update(svc, map);
	m_attacks.left_shockwave.handle_player(player);
	m_attacks.right_shockwave.handle_player(player);

	// gameplay logic
	p_health_bar.update(health.get_normalized());

	if (ccm::abs(Enemy::get_collider().physics.acceleration.x) > run_threshold_v) { request(LynxState::run); }
	if (Enemy::get_collider().get_center().x < m_home.x || Enemy::get_collider().get_center().x > m_home.y) {
		request(LynxState::jump);
		flags.state.set(StateFlags::out_of_zone);
	}
	if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { request(LynxState::turn); }
	if (player.is_dead()) {
		m_flags.set(LynxFlags::player_defeated);
		request(LynxState::laugh);
	}

	if (player.get_collider().bounding_box.overlaps(m_distant_range) && !was_introduced() && is_force_interact()) {
		set_distant_interact(true);
		svc.music_player.stop();
	}

	NPC::update(svc, map, *m_console, player);
	if (m_console->has_value() && was_introduced()) { set_force_interact(false); }

	if (!health.is_dead()) {
		// first phase starts
		if (b_lynx_start && !m_console->has_value()) {
			Boss::set_flag(BossFlags::battle_mode);
			request(LynxState::get_up);
			b_lynx_start = false;
			svc.music_player.load(svc.finder, "tumult");
			svc.music_player.play_looped();
			flags.general.set(GeneralFlags::has_invincible_channel);
			flags.state.set(StateFlags::vulnerable);
			svc.quest_table.progress_quest("lynx_dialogue", 1, 1);
			svc.data.save_quests();
		}
		// second phase starts
		if (half_health() && !Boss::has_flag_set(BossFlags::second_phase)) {
			request(LynxState::second_phase);
			set_distant_interact(true);
			set_force_interact(true);
			flush_conversations();
			push_conversation(3);
		}
	}
	if (Boss::has_flag_set(BossFlags::battle_mode)) { flags.state.set(StateFlags::vulnerable); }
	m_magic.deactivate();
	if (is_state(LynxState::downward_slam) || (is_state(LynxState::laugh) && !m_flags.test(LynxFlags::player_defeated))) {
		flags.state.reset(StateFlags::vulnerable);
		m_magic.activate();
	}
	if (is_levitating()) { m_magic.activate(); }
	if (health.is_dead()) {
		post_death.start(afterlife);
		flags.state.set(StateFlags::special_death_mode);
		request(LynxState::defeat);
	}
	state_function = state_function();
}

void Lynx::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	// NPC::render(win, cam);
	Enemy::render(svc, win, cam);
	if (b_lynx_debug) {
		for (auto& slash : m_attacks.slash) {
			if (slash.hit.active()) { slash.render(win, cam); }
		}
		m_attacks.left_shockwave.render(win, cam);
		m_attacks.right_shockwave.render(win, cam);
	}
	// m_magic.render(win, cam);
}

void Lynx::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	if (Boss::has_flag_set(BossFlags::battle_mode)) { p_health_bar.render(win); }
	if (b_lynx_debug) { debug(); }
}

fsm::StateFunction Lynx::update_sit() {
	m_state.actual = LynxState::sit;
	if (change_state(LynxState::get_up, Enemy::get_params("get_up"))) { return LYNX_BIND(update_get_up); }
	return LYNX_BIND(update_sit);
}

fsm::StateFunction Lynx::update_get_up() {
	m_state.actual = LynxState::get_up;
	Boss::set_flag(BossFlags::battle_mode);
	if (Enemy::animation.complete()) {
		if (change_state(LynxState::aerial_slash, Enemy::get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
		if (change_state(LynxState::forward_slash, Enemy::get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
		if (change_state(LynxState::triple_slash, Enemy::get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
		if (change_state(LynxState::upward_slash, Enemy::get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
		if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
		if (change_state(LynxState::levitate, Enemy::get_params("levitate"))) { return LYNX_BIND(update_levitate); }
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_get_up);
}

fsm::StateFunction Lynx::update_idle() {
	m_state.actual = LynxState::idle;
	m_counters.slam.cancel();
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (change_state(LynxState::laugh, Enemy::get_params("laugh"))) { return LYNX_BIND(update_laugh); }
	if (change_state(LynxState::turn, Enemy::get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (change_state(LynxState::aerial_slash, Enemy::get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
	if (change_state(LynxState::forward_slash, Enemy::get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
	if (change_state(LynxState::triple_slash, Enemy::get_params("triple_slash"))) { return LYNX_BIND(update_triple_slash); }
	if (change_state(LynxState::upward_slash, Enemy::get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
	if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	if (change_state(LynxState::run, Enemy::get_params("run"))) { return LYNX_BIND(update_run); }
	if (change_state(LynxState::jump, Enemy::get_params("jump"))) { return LYNX_BIND(update_jump); }
	if (change_state(LynxState::levitate, Enemy::get_params("levitate"))) { return LYNX_BIND(update_levitate); }
	if (change_state(LynxState::prepare_shuriken, Enemy::get_params("prepare_shuriken"))) { return LYNX_BIND(update_prepare_shuriken); }
	if (change_state(LynxState::prepare_slash, Enemy::get_params("prepare_slash"))) { return LYNX_BIND(update_prepare_slash); }
	return LYNX_BIND(update_idle);
}

fsm::StateFunction Lynx::update_jump() {
	m_state.actual = LynxState::jump;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }

	// check if battle is over and Lynx peaced out
	if (health.is_dead()) {
		flags.general.reset(GeneralFlags::map_collision);
		Enemy::get_collider().physics.acceleration.y = -80.f;
	} else {
		if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
			random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hah) : m_services->soundboard.flags.lynx.set(audio::Lynx::hoah);
			Enemy::get_collider().physics.acceleration.y = -260.f;
			Enemy::get_collider().physics.acceleration.x = Enemy::directions.actual.as_float() * 60.f;
		}
		if (Enemy::animation.get_frame_count() >= 3) { flags.general.reset(GeneralFlags::gravity); }
		if (Enemy::animation.complete()) {
			random::percent_chance(50) ? request(LynxState::aerial_slash) : request(LynxState::downward_slam);
			if (half_health()) { request(LynxState::aerial_slash); }
			if (flags.state.test(StateFlags::out_of_zone)) {
				request(LynxState::aerial_slash);
				flags.state.reset(StateFlags::out_of_zone);
			}
			if (change_state(LynxState::aerial_slash, Enemy::get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
			if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
		}
	}
	return LYNX_BIND(update_jump);
}

fsm::StateFunction Lynx::update_forward_slash() {
	m_state.actual = LynxState::forward_slash;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::heuh) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.get_frame_count() == 2 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{36.f * sign * static_cast<float>(i), static_cast<float>(i) * 4.f});
		i == 1 ? slash.set_constant_radius(28.f) : slash.set_constant_radius(18.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_forward_slash);
}

fsm::StateFunction Lynx::update_levitate() {
	m_state.actual = LynxState::levitate;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) {
		m_cooldowns.start_levitate.start();
		m_services->soundboard.flags.lynx.set(audio::Lynx::yyah);
	}
	if (m_cooldowns.start_levitate.is_complete() && !m_cooldowns.throw_shuriken.running()) { m_cooldowns.throw_shuriken.start(); }
	if (m_cooldowns.throw_shuriken.is_almost_complete() && !m_cooldowns.start_levitate.running()) {
		m_map->spawn_projectile_at(*m_services, m_shuriken.get(), m_shuriken.get().get_barrel_point(), m_attack_target);
		m_services->soundboard.flags.lynx.set(audio::Lynx::shing);
	}
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
		m_flags.set(LynxFlags::just_levitated);
		request(LynxState::downward_slam);
		if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_levitate);
}

fsm::StateFunction Lynx::update_run() {
	m_state.actual = LynxState::run;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	Enemy::get_collider().physics.acceleration.x = Enemy::directions.actual.as_float() * attributes.speed;
	if (Enemy::animation.just_started()) { m_cooldowns.run.start(); }
	if (change_state(LynxState::turn, Enemy::get_params("turn"))) { return LYNX_BIND(update_turn); }
	if (!m_cooldowns.run.running()) {
		request(LynxState::forward_slash);
		if (change_state(LynxState::forward_slash, Enemy::get_params("forward_slash"))) { return LYNX_BIND(update_forward_slash); }
	}
	return LYNX_BIND(update_run);
}

fsm::StateFunction Lynx::update_downward_slam() {
	m_state.actual = LynxState::downward_slam;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple, slam_follow());
	if (Enemy::animation.just_started()) { m_counters.slam.update(); }
	if (Enemy::animation.get_frame_count() == 7) { Enemy::get_collider().physics.acceleration.y = 200.f; }
	Enemy::animation.linger_on_frame(6, !Enemy::get_collider().grounded());
	// if (Enemy::animation.get_frame_count() >= 7 && !Enemy::get_collider().grounded()) { Enemy::animation.set_frame(7); }
	if (Enemy::animation.get_frame_count() == 4 && Enemy::animation.keyframe_started()) {
		m_map->spawn_effect(*m_services, "bright_flare", Enemy::get_collider().get_center() - sf::Vector2f{-Enemy::directions.actual.as_float(), 64.f}, sf::Vector2f{Enemy::get_collider().physics.velocity.x * 0.2f, 0.f}, 1);
	}
	if (Enemy::animation.get_frame_count() == 3 && Enemy::animation.keyframe_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::shing);
		m_services->soundboard.flags.lynx.set(audio::Lynx::hiyyah);
	}
	if (Enemy::animation.get_frame_count() == 8 && Enemy::animation.keyframe_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::slam);
		m_services->soundboard.flags.world.set(audio::World::thud);
		m_services->soundboard.flags.world.set(audio::World::vibration);
		m_map->spawn_emitter(*m_services, "fireworks", Enemy::get_collider().get_center(), Direction{UND::up}, {16.f, 16.f});
	}
	Enemy::animation.get_frame_count() < 8 ? flags.general.reset(GeneralFlags::gravity) : flags.general.set(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();

	// attack hitboxes
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		// downward slash
		if (Enemy::animation.get_frame_count() == 7) {
			slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{0.f, -20.f});
			slash.set_constant_radius(24.f);
			// ground slam
		} else if (Enemy::animation.get_frame_count() == 8) {
			auto offset = 24.f * (i - 1);
			slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{offset, 30.f});
			slash.set_constant_radius(i == 1 ? 32.f : 18.f);
		} else {
			slash.set_position(Enemy::get_collider().get_center());
			slash.set_constant_radius(8.f);
		}
	}

	if (Enemy::animation.get_frame_count() == 9) {
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		m_attacks.left_shockwave.start();
		m_attacks.right_shockwave.start();
	}
	if (Enemy::animation.complete()) {
		if (half_health() && m_counters.slam.get_count() < 3) {
			if (m_flags.test(LynxFlags::just_levitated)) {
				m_flags.reset(LynxFlags::just_levitated);
				request(LynxState::laugh);
				if (change_state(LynxState::laugh, Enemy::get_params("laugh"))) { return LYNX_BIND(update_laugh); }
			}
			flags.state.set(StateFlags::vulnerable);
			if (change_state(LynxState::turn, Enemy::get_params("turn"))) { return LYNX_BIND(update_turn); }
			request(LynxState::jump);
			if (change_state(LynxState::jump, Enemy::get_params("jump"))) { return LYNX_BIND(update_jump); }
		}
		flags.state.set(StateFlags::vulnerable);
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_downward_slam);
}

fsm::StateFunction Lynx::update_prepare_shuriken() {
	m_state.actual = LynxState::prepare_shuriken;
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::nngyah); }
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.complete()) {
		request(LynxState::toss_shuriken);
		if (change_state(LynxState::toss_shuriken, Enemy::get_params("toss_shuriken"))) { return LYNX_BIND(update_toss_shuriken); }
	}
	return LYNX_BIND(update_prepare_shuriken);
}

fsm::StateFunction Lynx::update_toss_shuriken() {
	m_state.actual = LynxState::toss_shuriken;
	m_cooldowns.post_shuriken_toss.start();
	if (Enemy::animation.just_started() || ((Enemy::animation.get_frame_count() == 0 || Enemy::animation.get_frame_count() == 3) && Enemy::animation.keyframe_started())) {
		m_map->spawn_projectile_at(*m_services, m_shuriken.get(), m_shuriken.get().get_barrel_point(), m_attack_target);
		m_services->soundboard.flags.lynx.set(audio::Lynx::shing);
	}
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_toss_shuriken);
}

fsm::StateFunction Lynx::update_upward_slash() {
	m_state.actual = LynxState::upward_slash;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hoah) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto& slash : m_attacks.slash) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{36.f * sign, -16.f});
		slash.set_constant_radius(60.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		if (change_state(LynxState::turn, Enemy::get_params("turn"))) { return LYNX_BIND(update_turn); }
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_upward_slash);
}

fsm::StateFunction Lynx::update_triple_slash() {
	m_state.actual = LynxState::triple_slash;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	auto sign = Enemy::directions.actual.as_float();
	for (auto& slash : m_attacks.slash) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{36.f * sign, -20.f});
		slash.set_constant_radius(64.f);
	}
	if (Enemy::animation.complete()) {
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_triple_slash);
}

fsm::StateFunction Lynx::update_turn() {
	m_state.actual = LynxState::turn;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::huuyeah); }
	Enemy::directions.desired.lock();
	if (Enemy::animation.get_frame_count() == 4 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{-28.f * sign * static_cast<float>(i), static_cast<float>(i) * -8.f});
		if (i == 0) { slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{16.f * sign, 8.f}); }
		slash.set_constant_radius(24.f);
		if (i == 1) { slash.set_constant_radius(32.f); }
	}
	if (Enemy::animation.complete()) {
		Enemy::request_flip();
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_turn);
}

fsm::StateFunction Lynx::update_aerial_slash() {
	m_state.actual = LynxState::aerial_slash;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) { random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::hoah) : m_services->soundboard.flags.lynx.set(audio::Lynx::huh); }
	if (Enemy::animation.just_started()) { Enemy::get_collider().physics.acceleration.x = Enemy::directions.actual.as_float() * 600.f; }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
	}
	flags.general.reset(GeneralFlags::gravity);
	auto sign = Enemy::directions.actual.as_float();
	for (auto [i, slash] : std::views::enumerate(m_attacks.slash)) {
		slash.set_position(Enemy::get_collider().get_center() + sf::Vector2f{36.f * sign * static_cast<float>(i), 0.f});
		i == 1 ? slash.set_constant_radius(20.f) : slash.set_constant_radius(16.f);
	}
	if (Enemy::animation.complete()) {
		flags.general.set(GeneralFlags::gravity);
		if (Enemy::get_collider().grounded()) {
			if (change_state(LynxState::turn, Enemy::get_params("turn"))) { return LYNX_BIND(update_turn); }
		}
		Enemy::get_collider().grounded() ? request(LynxState::upward_slash) : request(LynxState::downward_slam);
		if (change_state(LynxState::upward_slash, Enemy::get_params("upward_slash"))) { return LYNX_BIND(update_upward_slash); }
		if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_aerial_slash);
}

fsm::StateFunction Lynx::update_prepare_slash() {
	m_state.actual = LynxState::prepare_slash;
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (change_state(LynxState::second_phase, Enemy::get_params("second_phase"))) { return LYNX_BIND(update_second_phase); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::hnnyah); }
	if (Enemy::animation.get_frame_count() == 1 && Enemy::animation.keyframe_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::prepare);
		m_map->spawn_effect(*m_services, "lens_flare", Enemy::get_collider().get_center());
	}
	if (Enemy::animation.complete()) {
		request(LynxState::aerial_slash);
		if (change_state(LynxState::aerial_slash, Enemy::get_params("aerial_slash"))) { return LYNX_BIND(update_aerial_slash); }
	}
	return LYNX_BIND(update_prepare_slash);
}

fsm::StateFunction Lynx::update_defeat() {
	m_state.actual = LynxState::defeat;
	flags.general.set(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
	if (Enemy::animation.just_started()) {
		m_cooldowns.post_defeat.start();
		Boss::set_flag(BossFlags::battle_mode, false);
		set_distant_interact(true);
		set_force_interact(true);
		m_map->clear_projectiles();
		flush_conversations();
		push_conversation(2);
		m_services->music_player.pause();
		m_services->soundboard.flags.lynx.set(audio::Lynx::defeat);
	}
	if (m_cooldowns.post_defeat.is_almost_complete()) {
		m_map->spawn_emitter(*m_services, "white_fireworks", Enemy::get_collider().get_center(), Direction{UND::up});
		m_map->spawn_effect(*m_services, "small_explosion", Enemy::get_collider().get_center());
		flags.general.reset(GeneralFlags::post_death_render);
		m_services->soundboard.flags.enemy.set(audio::Enemy::disappear);
		post_death.cancel();
	}
	return LYNX_BIND(update_defeat);
}

fsm::StateFunction Lynx::update_second_phase() {
	m_state.actual = LynxState::second_phase;
	Boss::set_flag(BossFlags::second_phase);
	flags.state.reset(StateFlags::vulnerable);
	if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	if (Enemy::animation.just_started()) {
		m_services->soundboard.flags.lynx.set(audio::Lynx::laugh);
		Enemy::get_collider().physics.zero();
	}
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	Enemy::get_collider().physics.set_friction_componentwise(m_seek_friction);
	m_map->set_target_balance(0.f, audio::BalanceTarget::music);
	m_map->set_target_balance(0.f, audio::BalanceTarget::ambience);
	if (!m_console->has_value()) {
		m_services->music_player.load(m_services->finder, "tumultuous_spirit");
		m_services->music_player.play_looped();
		flags.general.set(GeneralFlags::gravity);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
		request(LynxState::downward_slam);
		if (change_state(LynxState::downward_slam, Enemy::get_params("downward_slam"))) { return LYNX_BIND(update_downward_slam); }
	}
	return LYNX_BIND(update_second_phase);
}

fsm::StateFunction Lynx::update_laugh() {
	m_state.actual = LynxState::laugh;
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.lynx.set(audio::Lynx::giggle); }
	if (m_flags.test(LynxFlags::player_defeated)) { return LYNX_BIND(update_laugh); }
	if (Enemy::animation.complete()) {
		m_cooldowns.post_levitate.start();
		request(LynxState::idle);
		if (change_state(LynxState::idle, Enemy::get_params("idle"))) { return LYNX_BIND(update_idle); }
	}
	return LYNX_BIND(update_laugh);
}

fsm::StateFunction Lynx::update_stagger() {
	m_state.actual = LynxState::stagger;
	if (Enemy::get_collider().grounded()) {
		request(LynxState::defeat);
		if (change_state(LynxState::defeat, Enemy::get_params("defeat"))) { return LYNX_BIND(update_defeat); }
	}
	return LYNX_BIND(update_stagger);
}

bool Lynx::change_state(LynxState next, anim::Parameters params) {
	if (m_state.desired == next) {
		Enemy::animation.set_params(params);
		return true;
	}
	return false;
}

void Lynx::debug() {
	static auto sz = ImVec2{180.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Lynx Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Post Shuriken: %i", m_cooldowns.post_shuriken_toss.get());
		ImGui::Text("Post Levitate: %i", m_cooldowns.post_levitate.get());
		ImGui::SeparatorText("Controls");
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
