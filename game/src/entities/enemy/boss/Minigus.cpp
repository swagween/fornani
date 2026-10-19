
#include "fornani/entities/enemy/boss/Minigus.hpp"
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Minigus::Minigus(automa::ServiceProvider& svc, world::Map& map, SceneContext& context)
	: Boss(svc, map, "minigus"), gun(svc, "minigun"), soda(svc, "soda_gun"), m_services(&svc), m_map(&map), sparkler(svc, Enemy::get_collider().get_vicinity_rect().size, colors::ui_white, "minigus"), m_context{&context}, m_minigun{svc},
	  jump_range{{400.f, 20.f}}, m_tattoo{svc, "minigus_tattoo", {118, 74}} {
	auto const fr = 30;
	p_animatable.set_animations({{"idle", {0, 4, fr * 2, -1}},		{"turn", {4, 3, fr, 0}},	 {"run", {7, 4, fr, 2}},		  {"shoot", {11, 2, fr, 2}},		  {"jump_shoot", {13, 2, fr, 2}}, {"jumpsquat", {15, 2, fr, 0}},
								 {"airborne", {17, 2, fr * 2, -1}}, {"land", {19, 2, fr, 0}},	 {"grab_soda", {21, 10, fr, 0}},  {"drink", {31, 2, fr, 3}},		  {"throw_can", {33, 6, fr, 0}},  {"stagger", {39, 1, fr * 3, 0}},
								 {"struggle", {40, 2, fr * 2, 3}},	{"recover", {42, 1, fr, 0}}, {"laugh", {43, 2, fr, 3}},		  {"punch", {45, 8, fr, 0}},		  {"snap", {53, 3, fr, 0}},		  {"reload", {56, 6, fr * 2, 0}},
								 {"wind_up", {62, 3, fr, 0}},		{"charge", {65, 4, fr, 3}},	 {"long_charge", {65, 4, fr, 2}}, {"charge_turn", {4, 3, fr * 2, 0}}, {"bump", {39, 1, fr * 2, 0}},	  {"exit", {15, 4, fr * 2, 0}}});

	p_animatable.animation.set_params(Enemy::get_params("idle"));
	gun.clip_cooldown_time = 360;
	gun.get().set_team(arms::Team::skycorps);
	soda.get().set_team(arms::Team::skycorps);
	gun.cycle.set_order(3);
	Enemy::get_collider().physics.maximum_velocity = {18.f, 28.f};
	Enemy::get_collider().physics.set_friction_componentwise({0.99f, 0.994f});
	afterlife = 2000;

	flags.general.set(GeneralFlags::post_death_render);
	flags.general.set(GeneralFlags::has_invincible_channel);

	m_tattoo.center();

	get_secondary_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_secondary_collider().set_dimensions({40.f, 40.f});
	m_minigun.center();
	m_minigun.push_and_set_animation("neutral", {0, 2, 48, -1});
	m_minigun.push_animation("firing", {2, 2, 16, -1});
	m_minigun.push_animation("cooldown", {4, 2, 32, 0});

	flags.state.set(StateFlags::vulnerable);

	attacks.punch.sensor.bounds.setRadius(75);
	attacks.punch.sensor.drawable.setFillColor(colors::blue);
	attacks.punch.hit.bounds.setRadius(50);
	attacks.punch.origin = {-10.f, -26.f};

	attacks.rush.sensor.bounds.setRadius(60);
	attacks.rush.hit.bounds.setRadius(60);

	distant_range.set_dimensions({720, 800});
	Enemy::get_collider().stats.GRAV = 6.0f;
	sprite_direction.lnr = LNR::left;
	Enemy::directions.actual.lnr = LNR::left;

	sparkler.set_dimensions(Enemy::get_collider().get_vicinity_rect().size);
	flags.state.set(StateFlags::invisible);
	auto const to_pos = sf::Vector2f{get_collider().get_position().x, 0.f};
	set_position(to_pos);
	m_minigun.set_position(to_pos);
	flags.state.set(StateFlags::no_shake);
	set_flag(BossFlags::ignore_oob);

	p_stagger_points = {0.25f, 0.5f, 0.75f};
}

void Minigus::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (init) {
		m_minigun.set_physics_position(Enemy::get_collider().get_center());
		init = false;
	}
	if (health.is_dead()) {
		post_death.start(afterlife);
		flags.state.set(StateFlags::special_death_mode);
	}
	sparkler.update(svc);
	sparkler.set_position(Enemy::get_collider().get_vicinity_rect().position);

	if (flags.state.test(StateFlags::invisible)) {
		auto const to_pos = sf::Vector2f{get_collider().get_position().x, -180.f};
		set_position(to_pos);
		m_minigun.set_position(to_pos);
	}
	if (flags.state.test(StateFlags::special_event)) { flags.state.reset(StateFlags::invisible); }

	if (map.off_the_bottom(Enemy::get_collider().physics.position)) {
		post_death.cancel();
		flags.general.set(GeneralFlags::no_loot);
	}

	gun.update(svc, map, *this);
	soda.get().set_firing_direction(directions.actual);
	soda.update(svc, map, *this);
	cooldowns.post_shot.update();
	cooldowns.aerial_shoot.update();
	cooldowns.long_charge.update();

	auto const center = get_collider().get_center();

	if (battle_mode()) {
		is_state(MinigusState::charge) || is_state(MinigusState::long_charge) ? attacks.rush.hit.activate() : attacks.rush.hit.deactivate();
		is_state(MinigusState::punch) && p_animatable.animation.get_frame_count() == 4 ? attacks.punch.hit.activate() : attacks.punch.hit.deactivate();
		attacks.rush.hurt_player(player, 1.f);
		if (attacks.punch.hurt_player(player, 2.f)) { map.spawn_frame_effect(*p_services, "punch", player.get_collider().get_center()); }
	}

	m_flags.set(MinigusFlags::show_minigun);
	if (is_state(MinigusState::stagger) || is_state(MinigusState::struggle) || is_state(MinigusState::punch)) { m_flags.reset(MinigusFlags::show_minigun); }

	sparkler.set_rate(flags.state.test(StateFlags::vulnerable) ? 0.f : 20.f);

	if (half_health() && secondary_collider) {
		auto pos = get_secondary_collider().physics.position + random::random_vector_float(10.f, 40.f);
		if (svc.ticker.every_x_ticks(10) && random::percent_chance(5)) { map.effects.push_back(entity::Effect(svc, "puff", pos, {0.f, -0.01f}, 3)); }
	}

	attacks.punch.update();
	attacks.punch.sensor.within_bounds(player.get_collider()) ? m_flags.set(MinigusFlags::player_is_punchable) : m_flags.reset(MinigusFlags::player_is_punchable);
	attacks.rush.update();
	jump_range.set_position(get_collider().get_bottom() - jump_range.get_dimensions() * 0.5f);
	auto const ro = sf::Vector2f{directions.actual.as_float() * 26.f, 0.f};
	attacks.rush.set_position(center + ro);
	attacks.punch.set_position(get_secondary_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 86.f, 54.f});
	for (auto& s : attacks.shockwaves) {
		s.origin = get_collider().get_bottom();
		s.update(svc, map);
		s.hurt_player(player);
	}

	if (battle_mode()) { attacks.punch.handle_player(player); }
	for (auto& breakable : map.breakables) {
		if (Enemy::get_collider().jumpbox.overlaps(breakable->get_bounding_box())) { breakable->on_smash(svc, map, 4); }
		if ((is(MinigusState::charge) || is_state(MinigusState::long_charge)) && attacks.rush.hit.within_bounds(breakable->get_bounding_box())) {
			if (svc.ticker.every_x_ticks(18)) { breakable->on_smash(svc, map, 1); }
		}
		if (attacks.punch.hit.active() && attacks.punch.hit.within_bounds(breakable->get_bounding_box())) {
			if (svc.ticker.every_x_ticks(2)) { breakable->on_smash(svc, map, 4); }
		}
	}

	// minigun
	m_gun_offset = {43.f, 23.f};
	auto const frame = p_animatable.get_frame();
	if (frame == 9) { m_gun_offset = {21.f, 13.f}; }
	if (frame == 7) { m_gun_offset = {49.f, 13.f}; }
	if (frame == 36) { m_gun_offset = {27.f, 17.f}; }
	if (frame == 42) { m_gun_offset = {41.f, 27.f}; }
	if (frame == 56) { m_gun_offset = {47.f, 7.f}; }
	if (frame == 57) { m_gun_offset = {47.f, 7.f}; }
	if (frame == 58) { m_gun_offset = {47.f, 7.f}; }
	if (frame == 62) { m_gun_offset = {17.f, 15.f}; }
	if (frame == 63) { m_gun_offset = {19.f, 13.f}; }
	if (frame == 64) { m_gun_offset = {21.f, 11.f}; }
	m_gun_offset.x *= directions.actual.as_float();

	m_minigun.update(Enemy::get_collider().get_center() + m_gun_offset);
	if (m_minigun.animation.is_complete()) {
		if (m_minigun.get_animation_tag() == "cooldown") { m_minigun.set_animation("neutral"); }
	}

	auto const soda_point = get_secondary_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 12.f, 0.f};
	soda.get().set_barrel_point(soda_point);
	auto const gun_point = center + sf::Vector2f{directions.actual.as_float() * 40.f, 20.f + 8.f * m_barrel_cycle.get_normalized()};
	gun.get().set_barrel_point(gun_point);

	if (is_alert()) {
		auto const hp = map.get_closest_home_point(center);
		auto const dir = center.x > hp.x ? Direction{LR::left} : Direction{LR::right};
		directions.desired = dir;
	} else {
		face_player(player);
	}
	Boss::update(svc, map, player);

	if (secondary_collider) {
		auto const soffset = is_state(MinigusState::struggle) ? sf::Vector2f{0.f, 0.f} : sf::Vector2f{0.f, -70.f};
		get_secondary_collider().set_position(center - get_secondary_collider().get_local_center() + soffset);
	}

	// state management
	auto const manual = false;
	if (!manual) {
		if (svc.ticker.every_second()) { random::coin_flip() ? request(MinigusState::run) : cooldowns.post_shot.running() ? request(MinigusState::jumpsquat) : request(MinigusState::reload); }
		if (is_alert()) {
			request(MinigusState::run);
		} else if (player.get_collider().bounding_box.overlaps(jump_range)) {
			request(MinigusState::jumpsquat);
		}

		if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { is_state(MinigusState::long_charge) ? request(MinigusState::charge_turn) : request(MinigusState::turn); }
		movement_direction.lnr = Enemy::get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;

		if (!battle_mode()) { request(MinigusState::idle); }
		if (!get_collider().grounded()) { request(MinigusState::airborne); }

		if (!has_flag_set(BossFlags::second_phase) && half_health()) { set_flag(BossFlags::second_phase); }
		if (is_staggered()) { request(MinigusState::stagger); }
	}

	if (player.is_dead()) {
		request(MinigusState::laugh);
		set_flag(BossFlags::defeated_player);
	}

	console_complete = !m_context->console.has_value();

	if (Boss::consume_flag(BossFlags::start_battle)) {
		svc.quest_table.progress_quest("minigus_dialogue", 1, 1);
		svc.data.save_dialogue_quests();
	}

	if (m_flags.test(MinigusFlags::defeated) && !m_flags.test(MinigusFlags::dropped_loot)) {
		map.active_loot.push_back(item::Loot(svc, map, player, Enemy::get_collider().get_center(), {get_attributes().drop_range, get_attributes().loot_multiplier, 0, false, 0, 16}));
		m_flags.set(MinigusFlags::dropped_loot);
	}

	if (health.is_dead() && !m_flags.test(MinigusFlags::defeated)) { request(MinigusState::stagger); }

	Minigus::state_function = Minigus::state_function();
}

void Minigus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);

	m_minigun.set_scale(p_animatable.get_scale());
	if (flags.state.test(StateFlags::invisible)) { return; }
	if (directions.actual.left()) {
		m_tattoo.set_position(p_animatable.get_window_position());
		m_tattoo.set_frame(p_animatable.get_frame());
		win.draw(m_tattoo);
	}
	m_minigun.render(cam);
	sparkler.render(win, cam);

	if (debug::is_production()) {
		if (m_flags.test(MinigusFlags::show_minigun)) { win.draw(m_minigun); }
		// sprite_history.drag(win, cam);
	} else {
		// distant_range.render(win, cam);
	}
	// if (is(MinigusState::punch)) { attacks.punch.render(win, cam); }
	// attacks.rush.render(win, cam);
	// jump_range.render(win, cam);
}

void Minigus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	debug();
}

fsm::StateFunction Minigus::update_idle() {
	p_state.actual = MinigusState::idle;
	if (m_flags.test(MinigusFlags::defeated)) {
		if (m_context->cutscene_catalog.cutscenes.empty()) {
			p_services->music_player.load(p_services->finder, "dusken");
			p_services->music_player.play_looped();
			request(MinigusState::exit);
			if (change_state(MinigusState::exit, get_params("exit"))) { return MINIGUS_BIND(update_exit); }
		}
		return MINIGUS_BIND(update_idle);
	}
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {}
	if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
	if (change_state(MinigusState::run, get_params("run"))) { return MINIGUS_BIND(update_run); }
	if (change_state(MinigusState::snap, get_params("snap"))) { return MINIGUS_BIND(update_snap); }
	if (change_state(MinigusState::reload, get_params("reload"))) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::jumpsquat, get_params("jumpsquat"))) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::grab_soda, get_params("grab_soda"))) { return MINIGUS_BIND(update_grab_soda); }
	if (change_state(MinigusState::punch, get_params("punch"))) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::wind_up, get_params("wind_up"))) { return MINIGUS_BIND(update_wind_up); }
	if (change_state(MinigusState::laugh, get_params("laugh"))) { return MINIGUS_BIND(update_laugh); }
	if (p_animatable.animation.complete()) {
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_idle);
}

fsm::StateFunction Minigus::update_turn() {
	p_state.actual = MinigusState::turn;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		request_flip();
		if (is_alert()) {
			request(MinigusState::run);
			if (change_state(MinigusState::run, get_params("run"))) { return MINIGUS_BIND(update_run); }
		}
		is_hostile() ? request(MinigusState::punch) : cooldowns.post_shot.running() ? request(MinigusState::grab_soda) : request(MinigusState::reload);
		if (change_state(MinigusState::reload, get_params("reload"))) { return MINIGUS_BIND(update_reload); }
		if (change_state(MinigusState::punch, get_params("punch"))) { return MINIGUS_BIND(update_punch); }
		if (change_state(MinigusState::grab_soda, get_params("grab_soda"))) { return MINIGUS_BIND(update_grab_soda); }
	}
	return MINIGUS_BIND(update_turn);
}

fsm::StateFunction Minigus::update_run() {
	p_state.actual = MinigusState::run;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {}
	auto const frame = p_animatable.animation.get_frame_count();
	if ((frame == 1 || frame == 3) && p_animatable.animation.keyframe_started()) {
		p_services->soundboard.play_sound("tank_step", get_collider().get_center());
		p_services->camera_controller.shake(4, 0.1f, 200, 20);
	}
	get_collider().physics.velocity.x = attributes.speed * directions.actual.as_float();
	if (p_animatable.animation.complete()) {
		if (m_flags.test(MinigusFlags::player_is_punchable)) {
			request(MinigusState::punch);
			if (change_state(MinigusState::punch, get_params("punch"))) { return MINIGUS_BIND(update_punch); }
		}
		auto const l = get_collider().has_left_wallslide_collision() && directions.actual.left();
		auto const r = get_collider().has_right_wallslide_collision() && directions.actual.right();
		if ((l || r) && !is_facing_player()) { request(MinigusState::turn); }
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		if (l || r) {
			request(MinigusState::jumpsquat);
			m_flags.set(MinigusFlags::short_hop);
		}
		if (change_state(MinigusState::jumpsquat, get_params("jumpsquat"))) { return MINIGUS_BIND(update_jumpsquat); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_run);
}

fsm::StateFunction Minigus::update_shoot() {
	p_state.actual = MinigusState::shoot;
	if (change_state(MinigusState::stagger, get_params("stagger"))) {
		m_minigun.set_animation("cooldown");
		return MINIGUS_BIND(update_stagger);
	}
	if (change_state(MinigusState::airborne, get_params("airborne"))) {
		m_minigun.set_animation("cooldown");
		return MINIGUS_BIND(update_airborne);
	}
	shoot();
	if (p_animatable.animation.just_started()) { m_minigun.set_animation("firing"); }
	if (p_animatable.animation.complete()) {
		m_minigun.set_animation("cooldown");
		if (has_flag_set(BossFlags::second_phase)) {
			m_flags.set(MinigusFlags::aerial_shoot);
			request(MinigusState::reload);
			if (change_state(MinigusState::reload, get_params("reload"))) { return MINIGUS_BIND(update_reload); }
		}
		if (m_flags.test(MinigusFlags::player_is_punchable)) {
			request(MinigusState::punch);
			if (change_state(MinigusState::punch, get_params("punch"))) { return MINIGUS_BIND(update_punch); }
		}
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_shoot);
}

fsm::StateFunction Minigus::update_jump_shoot() {
	p_state.actual = MinigusState::jump_shoot;
	if (change_state(MinigusState::stagger, get_params("stagger"))) {
		m_minigun.set_animation("cooldown");
		return MINIGUS_BIND(update_stagger);
	}
	if (cooldowns.aerial_shoot.is_almost_complete()) { m_minigun.set_animation("firing"); }
	if (cooldowns.aerial_shoot.is_complete()) { shoot(); }
	get_collider().physics.velocity.x = directions.actual.as_float() * -3.f;
	if (p_animatable.animation.just_started()) { m_flags.reset(MinigusFlags::aerial_shoot); }
	if (p_animatable.animation.complete()) {
		m_minigun.set_animation("cooldown");
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_jump_shoot);
}

fsm::StateFunction Minigus::update_jumpsquat() {
	p_state.actual = MinigusState::jumpsquat;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (p_animatable.animation.complete()) {
		if (m_flags.test(MinigusFlags::aerial_shoot) && !get_collider().has_left_wallslide_collision() && !get_collider().has_right_wallslide_collision()) {
			get_collider().physics.velocity.x = directions.actual.as_float() * -4.f;
			get_collider().physics.velocity.y = -20.f;
			p_services->soundboard.play_sound("minigus_momma", get_collider().get_center());
			cooldowns.aerial_shoot.start();
			request(MinigusState::jump_shoot);
			if (change_state(MinigusState::jump_shoot, get_params("jump_shoot"))) { return MINIGUS_BIND(update_jump_shoot); }
		}
		if (m_flags.consume(MinigusFlags::short_hop)) {
			get_collider().physics.velocity.x = directions.actual.as_float() * -28.f;
			get_collider().physics.velocity.y = -10.f;
			p_services->soundboard.play_sound("minigus_ok_1", get_collider().get_center());
			request(MinigusState::airborne);
			if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
		}
		get_collider().physics.velocity.x = directions.actual.as_float() * 18.f;
		get_collider().physics.velocity.y = -20.f;
		p_services->soundboard.play_sound("minigus_woob", get_collider().get_center());
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_jumpsquat);
}

fsm::StateFunction Minigus::update_airborne() {
	p_state.actual = MinigusState::airborne;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (p_animatable.animation.just_started()) {}
	if (get_collider().grounded()) {
		request(MinigusState::land);
		if (change_state(MinigusState::land, get_params("land"))) { return MINIGUS_BIND(update_land); }
	}
	return MINIGUS_BIND(update_airborne);
}

fsm::StateFunction Minigus::update_land() {
	p_state.actual = MinigusState::land;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {
		p_services->camera_controller.shake(10, 0.4f, 200, 20);
		p_services->soundboard.play_sound("vibration", get_collider().get_center());
		p_services->soundboard.play_sound("heavy_land", get_collider().get_center());
		if (battle_mode()) {
			auto const shockwave_speed = 0.8f;
			attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 3, {shockwave_speed * directions.actual.as_float(), 0.f}, false}});
			attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
			attacks.shockwaves.back().start();
			attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 3, {-shockwave_speed * directions.actual.as_float(), 0.f}, false}});
			attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
			attacks.shockwaves.back().start();
		}
		m_map->spawn_effect(*p_services, "strong_slam", get_collider().get_center() + sf::Vector2f{0.f, 28.f});
	}
	if (p_animatable.animation.complete()) {
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_land);
}

fsm::StateFunction Minigus::update_grab_soda() {
	p_state.actual = MinigusState::grab_soda;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_deepspeak", get_collider().get_center()); }
	if (p_animatable.frame_action(6)) { p_services->soundboard.play_sound("minigus_soda", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		request(MinigusState::drink);
		if (change_state(MinigusState::drink, get_params("drink"))) { return MINIGUS_BIND(update_drink); }
	}
	return MINIGUS_BIND(update_grab_soda);
}

fsm::StateFunction Minigus::update_drink() {
	p_state.actual = MinigusState::drink;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		p_services->soundboard.play_sound("minigus_invincible", get_collider().get_center());
		flags.state.reset(StateFlags::vulnerable);
		request(MinigusState::throw_can);
		if (change_state(MinigusState::throw_can, get_params("throw_can"))) { return MINIGUS_BIND(update_throw_can); }
	}
	return MINIGUS_BIND(update_drink);
}

fsm::StateFunction Minigus::update_throw_can() {
	p_state.actual = MinigusState::throw_can;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_poh", get_collider().get_center()); }
	if (p_animatable.frame_action(3)) { soda.shoot(*p_services, *p_map); }
	if (p_animatable.animation.complete()) {
		request(MinigusState::wind_up);
		if (change_state(MinigusState::wind_up, get_params("wind_up"))) { return MINIGUS_BIND(update_wind_up); }
	}
	return MINIGUS_BIND(update_throw_can);
}

fsm::StateFunction Minigus::update_stagger() {
	p_state.actual = MinigusState::stagger;
	if (p_animatable.animation.just_started()) {
		get_collider().physics.velocity.x = directions.actual.as_float() * -12.f;
		get_collider().physics.velocity.y = -8.f;
		p_services->soundboard.play_sound("minigus_quick_breath", get_collider().get_center());
		m_services->ticker.freeze_frame(0.8f);
		p_map->spawn_frame_effect(*m_services, "impact_frame", get_collider().get_center());
		p_map->spawn_frame_effect(*m_services, "hurt_spark", get_secondary_collider().get_center());
	}
	shake();
	if (p_animatable.animation.complete()) {
		request(MinigusState::struggle);
		if (change_state(MinigusState::struggle, get_params("struggle"))) { return MINIGUS_BIND(update_struggle); }
	}
	return MINIGUS_BIND(update_stagger);
}

fsm::StateFunction Minigus::update_struggle() {
	p_state.actual = MinigusState::struggle;
	if (p_animatable.animation.just_started()) {
		p_services->soundboard.play_sound("minigus_exhale", get_collider().get_center());
		if (health.is_dead() && !m_context->console) {
			Boss::set_flag(BossFlags::battle_mode, false);
			m_services->quest_table.set_quest_progression("defeat_minigus", 2);
			m_services->events.launch_cutscene_event.dispatch(*m_services, 117);
			m_map->clear_projectiles();
		}
	}
	flags.general.reset(GeneralFlags::player_collision);
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		if (health.is_dead()) {
			m_flags.set(MinigusFlags::defeated);
			if (!m_context->console) {
				request(MinigusState::idle);
				if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
			}
			request(MinigusState::struggle);
			if (change_state(MinigusState::struggle, get_params("struggle"))) { return MINIGUS_BIND(update_struggle); }
		}

		flags.general.set(GeneralFlags::player_collision);
		request(MinigusState::recover);
		if (change_state(MinigusState::recover, get_params("recover"))) { return MINIGUS_BIND(update_recover); }
	}
	return MINIGUS_BIND(update_struggle);
}

fsm::StateFunction Minigus::update_recover() {
	p_state.actual = MinigusState::recover;
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_dontlookatme", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		request(MinigusState::grab_soda);
		if (change_state(MinigusState::grab_soda, get_params("grab_soda"))) { return MINIGUS_BIND(update_grab_soda); }
	}
	return MINIGUS_BIND(update_recover);
}

fsm::StateFunction Minigus::update_laugh() {
	p_state.actual = MinigusState::laugh;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started() && p_animatable.animation.is_first_loop()) { p_services->soundboard.play_sound("minigus_laugh_1", get_collider().get_center()); }
	if (p_animatable.animation.complete() && !has_flag_set(BossFlags::defeated_player)) {
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_laugh);
}

fsm::StateFunction Minigus::update_punch() {
	p_state.actual = MinigusState::punch;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_ok_2", get_collider().get_center()); }
	if (p_animatable.frame_action(4)) { p_services->soundboard.play_sound("minigus_punch", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() < 4) {
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
	}
	if (p_animatable.animation.complete()) {
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_punch);
}

fsm::StateFunction Minigus::update_snap() {
	p_state.actual = MinigusState::snap;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_snap", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		for (int i{0}; i < 2; ++i) {
			auto randx = random::random_range_float(-80.f, 80.f);
			auto randy = random::random_range_float(-160.f, 0.f);
			sf::Vector2f rand_vec{randx, randy};
			sf::Vector2f spawn = get_collider().get_center() + rand_vec;
			m_map->spawn_enemy(5, spawn);
		}
		request(MinigusState::laugh);
		if (change_state(MinigusState::laugh, get_params("laugh"))) { return MINIGUS_BIND(update_laugh); }
	}
	return MINIGUS_BIND(update_snap);
}

fsm::StateFunction Minigus::update_reload() {
	p_state.actual = MinigusState::reload;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_pizza", get_collider().get_center()); }
	if (p_animatable.animation.get_frame_count() < 3) { shake(); }
	if (p_animatable.frame_action(3)) {
		p_services->ticker.freeze_frame(0.3f);
		p_services->soundboard.play_sound("minigus_mechanism", get_collider().get_center());
	}
	if (p_animatable.animation.complete()) {
		cooldowns.post_shot.start();
		if (m_flags.test(MinigusFlags::aerial_shoot)) {
			request(MinigusState::jumpsquat);
			if (change_state(MinigusState::jumpsquat, get_params("jumpsquat"))) { return MINIGUS_BIND(update_jumpsquat); }
		}
		request(MinigusState::shoot);
		if (change_state(MinigusState::shoot, get_params("shoot"))) { return MINIGUS_BIND(update_shoot); }
	}
	return MINIGUS_BIND(update_reload);
}

fsm::StateFunction Minigus::update_wind_up() {
	p_state.actual = MinigusState::wind_up;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_itsagreatday", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
		if (has_flag_set(BossFlags::second_phase)) {
			cooldowns.long_charge.start();
			request(MinigusState::long_charge);
			if (change_state(MinigusState::long_charge, get_params("long_charge"))) { return MINIGUS_BIND(update_long_charge); }
		}
		request(MinigusState::charge);
		if (change_state(MinigusState::charge, get_params("charge"))) { return MINIGUS_BIND(update_charge); }
	}
	return MINIGUS_BIND(update_wind_up);
}

fsm::StateFunction Minigus::update_charge() {
	p_state.actual = MinigusState::charge;
	get_collider().physics.acceleration.x = directions.actual.as_float() * 2.f;
	auto const frame = p_animatable.animation.get_frame_count();
	if ((frame == 1 || frame == 3) && p_animatable.animation.keyframe_started()) {
		p_services->soundboard.play_sound("tank_step", get_collider().get_center());
		p_services->camera_controller.shake(4, 0.1f, 200, 20);
	}
	flags.general.reset(GeneralFlags::player_collision);
	if (p_animatable.animation.just_started()) {}
	if (change_state(MinigusState::stagger, get_params("stagger"))) {
		flags.general.set(GeneralFlags::player_collision);
		return MINIGUS_BIND(update_stagger);
	}
	if (change_state(MinigusState::airborne, get_params("airborne"))) {
		flags.general.set(GeneralFlags::player_collision);
		flags.state.set(StateFlags::vulnerable);
		return MINIGUS_BIND(update_airborne);
	}
	if (has_side_collision()) {
		flags.general.set(GeneralFlags::player_collision);
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::bump);
		if (change_state(MinigusState::bump, get_params("bump"))) { return MINIGUS_BIND(update_bump); }
	}
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		flags.general.set(GeneralFlags::player_collision);
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::laugh);
		if (change_state(MinigusState::laugh, get_params("laugh"))) { return MINIGUS_BIND(update_laugh); }
	}
	return MINIGUS_BIND(update_charge);
}

fsm::StateFunction Minigus::update_long_charge() {
	p_state.actual = MinigusState::long_charge;
	get_collider().physics.acceleration.x = directions.actual.as_float() * 2.f;
	auto const frame = p_animatable.animation.get_frame_count();
	if ((frame == 1 || frame == 3) && p_animatable.animation.keyframe_started()) {
		p_services->soundboard.play_sound("tank_step", get_collider().get_center());
		p_services->camera_controller.shake(4, 0.1f, 200, 20);
	}
	flags.general.reset(GeneralFlags::player_collision);
	if (p_animatable.animation.just_started()) {}
	if (change_state(MinigusState::stagger, get_params("stagger"))) {
		flags.general.set(GeneralFlags::player_collision);
		return MINIGUS_BIND(update_stagger);
	}
	if (has_side_collision()) {
		flags.general.set(GeneralFlags::player_collision);
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::bump);
		if (change_state(MinigusState::bump, get_params("bump"))) { return MINIGUS_BIND(update_bump); }
	}
	if (change_state(MinigusState::charge_turn, get_params("charge_turn"))) { return MINIGUS_BIND(update_charge_turn); }
	if (cooldowns.long_charge.is_complete() || has_flag_set(BossFlags::defeated_player)) {
		flags.general.set(GeneralFlags::player_collision);
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::laugh);
		if (change_state(MinigusState::laugh, get_params("laugh"))) { return MINIGUS_BIND(update_laugh); }
	}
	return MINIGUS_BIND(update_long_charge);
}

fsm::StateFunction Minigus::update_charge_turn() {
	p_state.actual = MinigusState::charge_turn;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		request_flip();
		request(MinigusState::long_charge);
		if (change_state(MinigusState::long_charge, get_params("long_charge"))) { return MINIGUS_BIND(update_long_charge); }
	}
	return MINIGUS_BIND(update_charge_turn);
}

fsm::StateFunction Minigus::update_exit() {
	p_state.actual = MinigusState::exit;
	if (p_animatable.animation.just_started()) {
		get_collider().physics.velocity.y = -8.f;
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	}
	if (p_animatable.animation.complete()) {
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_exit);
}

fsm::StateFunction Minigus::update_bump() {
	p_state.actual = MinigusState::bump;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (p_animatable.animation.just_started()) {
		m_map->spawn_effect(*p_services, "strong_side_slam", get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 28.f, 0.f}, {}, 0, directions.actual.as_float());
		get_collider().physics.velocity.x = directions.actual.as_float() * -14.f;
		get_collider().physics.velocity.y = -10.f;
		p_services->soundboard.play_sound("delay_crash", get_collider().get_center());
		p_services->soundboard.play_sound("minigus_poh", get_collider().get_center());
		p_services->camera_controller.shake(10, 0.4f, 200, 20);
	}
	if (p_animatable.animation.complete()) {
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_bump);
}

void Minigus::debug() {
	static auto sz = ImVec2{280.f, 350.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Minigus Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("Post Shot: ");
		ImGui::ProgressBar(cooldowns.post_shot.get_normalized());
		ImGui::Text("Facing Player? %s ", is_facing_player() ? "Yes" : "No");
		ImGui::Text("Defeated? %s ", m_flags.test(MinigusFlags::defeated) ? "Yes" : "No");
		ImGui::Text("Desired Direction %s ", directions.desired.left() ? "Left" : "Right");
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("idle")) { request(MinigusState::idle); }
		if (ImGui::Button("run")) { request(MinigusState::run); }
		if (ImGui::Button("shoot")) { request(MinigusState::shoot); }
		if (ImGui::Button("jump_shoot")) {
			m_flags.set(MinigusFlags::aerial_shoot);
			request(MinigusState::jumpsquat);
		}
		if (ImGui::Button("jumpsquat")) { request(MinigusState::jumpsquat); }
		if (ImGui::Button("grab_soda")) { request(MinigusState::grab_soda); }
		if (ImGui::Button("stagger")) { request(MinigusState::stagger); }
		if (ImGui::Button("struggle")) { request(MinigusState::struggle); }
		if (ImGui::Button("recover")) { request(MinigusState::recover); }
		if (ImGui::Button("laugh")) { request(MinigusState::laugh); }
		if (ImGui::Button("snap")) { request(MinigusState::snap); }
		if (ImGui::Button("reload")) { request(MinigusState::reload); }
		if (ImGui::Button("wind_up")) { request(MinigusState::wind_up); }
		ImGui::End();
	}
}

auto Minigus::has_side_collision() const -> bool {
	auto const left = get_collider().bounding_box.vertices[0] - sf::Vector2f{1.f, 0.f};
	auto const right = get_collider().bounding_box.vertices[1] + sf::Vector2f{1.f, 0.f};
	auto const oll = m_map->overlaps_middleground(left) && directions.actual.left();
	auto const olr = m_map->overlaps_middleground(right) && directions.actual.right();
	return olr || oll;
}

void Minigus::shoot() {
	if (p_services->ticker.every_x_ticks(10)) {
		gun.shoot(*p_services, *p_map);
		m_barrel_cycle.update();
	}
}

bool Minigus::change_state(MinigusState next, anim::Parameters params) {
	if (StateMachine<MinigusState>::p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

void Minigun::update(sf::Vector2f const target) {
	tick();
	m_steering.seek(m_physics, target, 0.006f);
	m_physics.simple_update();
}

void Minigun::render(sf::Vector2f const cam) { set_position(m_physics.position - cam); }

} // namespace fornani::enemy
