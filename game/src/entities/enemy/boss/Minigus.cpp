
#include "fornani/entities/enemy/boss/Minigus.hpp"
#include <fornani/automa/SceneContext.hpp>
#include <fornani/core/Debug.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Minigus::Minigus(automa::ServiceProvider& svc, world::Map& map, SceneContext& context)
	: Boss(svc, map, "minigus"), gun(svc, "minigun"), soda(svc, "soda_gun"), m_services(&svc), m_map(&map), sparkler(svc, Enemy::get_collider().get_vicinity_rect().size, colors::ui_white, "minigus"), m_context{&context},
	  m_mode{MinigusMode::neutral}, m_minigun{svc} {
	auto const fr = 32;
	p_animatable.set_animations({{"idle", {0, 4, fr * 2, -1}},		{"turn", {4, 3, fr, 0}},	 {"run", {7, 4, fr, 3}},		 {"shoot", {11, 2, fr, 3}}, {"jump_shoot", {13, 2, fr, 3}}, {"jumpsquat", {15, 2, fr, 0}},
								 {"airborne", {17, 2, fr * 2, -1}}, {"land", {19, 2, fr, 0}},	 {"grab_soda", {21, 10, fr, 0}}, {"drink", {31, 2, fr, 3}}, {"throw_can", {33, 6, fr, 0}},	{"stagger", {39, 1, fr, 0}},
								 {"struggle", {40, 2, fr * 3, 3}},	{"recover", {42, 1, fr, 0}}, {"laugh", {43, 2, fr, 3}},		 {"punch", {45, 8, fr, 0}}, {"snap", {53, 3, fr, 0}},		{"reload", {56, 6, fr * 3, 0}},
								 {"wind_up", {62, 3, fr, 0}},		{"charge", {65, 4, fr, 2}},	 {"exit", {15, 4, fr, 0}}});

	p_animatable.animation.set_params(Enemy::get_params("idle"));
	gun.clip_cooldown_time = 360;
	gun.get().set_team(arms::Team::skycorps);
	soda.get().set_team(arms::Team::skycorps);
	gun.cycle.set_order(3);
	Enemy::get_collider().physics.maximum_velocity = {8.f, 18.f};
	Enemy::get_collider().physics.set_constant_friction({0.99f, 0.989f});
	cooldowns.vulnerability.start();
	cooldowns.exit.start();
	afterlife = 2000;

	flags.general.set(GeneralFlags::post_death_render);
	flags.general.set(GeneralFlags::has_invincible_channel);

	get_secondary_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_secondary_collider().set_dimensions({40.f, 40.f});
	m_minigun.center();
	m_minigun.set_parameters(m_minigun.neutral);
	flags.state.set(StateFlags::vulnerable);

	attacks.punch.sensor.bounds.setRadius(60);
	attacks.punch.sensor.drawable.setFillColor(colors::blue);
	attacks.punch.hit.bounds.setRadius(32);
	attacks.punch.origin = {-10.f, -26.f};

	attacks.rush.sensor.bounds.setRadius(60);
	attacks.rush.hit.bounds.setRadius(60);
	attacks.rush.origin = {40.f, 10.f};

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
		request(MinigusState::stagger);
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
	cooldowns.firing.update();
	cooldowns.post_charge.update();
	cooldowns.post_punch.update();
	cooldowns.hurt.update();
	cooldowns.hurt_sound.update();
	cooldowns.player_punch.update();

	if (battle_mode()) {
		cooldowns.vulnerability.update();
		is_state(MinigusState::charge) ? attacks.rush.hit.deactivate() : attacks.rush.hit.activate();
		attacks.rush.hurt_player(player, 1.f);
	}

	attacks.punch.update();
	attacks.rush.update();
	for (auto& s : attacks.shockwaves) {
		s.origin = get_collider().get_bottom();
		s.update(svc, map);
	}

	if (battle_mode()) { attacks.punch.handle_player(player); }
	for (auto& breakable : map.breakables) {
		if (Enemy::get_collider().jumpbox.overlaps(breakable->get_bounding_box())) { breakable->on_smash(svc, map, 4); }
		if (is(MinigusState::charge) && attacks.rush.hit.within_bounds(breakable->get_bounding_box())) {
			if (svc.ticker.every_x_ticks(18)) { breakable->on_smash(svc, map, 1); }
		}
	}
	auto const center = get_collider().get_center();
	m_minigun.update(Enemy::get_collider().get_center());
	m_minigun.offset = sf::Vector2f{directions.actual.as_float() * 32.f, 0.f};

	auto const soda_point = get_secondary_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 12.f, 0.f};
	soda.get().set_barrel_point(soda_point);
	auto const gun_point = center + sf::Vector2f{directions.actual.as_float() * 40.f, 12.f + 8.f * m_barrel_cycle.get_normalized()};
	gun.get().set_barrel_point(gun_point);

	if (is_alert()) {
		auto const hp = map.get_closest_home_point(center);
		auto const dir = center.x > hp.x ? SimpleDirection{LR::left} : SimpleDirection{LR::right};
		directions.desired = dir;
	} else {
		face_player(player);
	}
	Boss::update(svc, map, player);

	if (secondary_collider) { get_secondary_collider().set_position(center - get_secondary_collider().get_local_center() - sf::Vector2f{0.f, 70.f}); }

	// state management
	if (svc.ticker.every_second()) { random::coin_flip() ? request(MinigusState::run) : request(MinigusState::reload); }
	if (is_alert()) {
		request(MinigusState::run);
	} else if (is_hostile()) {
		request(MinigusState::jumpsquat);
	}

	// old stuff
	auto const manual = true;
	if (!manual) {
		if (attacks.punch.sensor.active() && cooldowns.post_punch.is_complete()) { request(MinigusState::punch); }

		if (m_minigun.flags.test(MinigunFlags::exhausted) && cooldowns.firing.is_complete()) {
			if (random::percent_chance(12)) { request(MinigusState::reload); }
		}

		if (gun.clip_cooldown.is_complete() && !m_minigun.flags.test(MinigunFlags::exhausted) && !cooldowns.post_charge.running() && is_hostile()) {
			if (random::percent_chance(fire_chance)) {
				if (random::percent_chance(50)) {
					request(MinigusState::jump_shoot);
				} else {
					request(MinigusState::shoot);
				}
			}
		}

		if (player.get_collider().bounding_box.overlaps(Enemy::get_collider().get_vicinity_rect())) {
			if (random::percent_chance(30)) {
				request(MinigusState::run);
			} else {
				request(MinigusState::jumpsquat);
			}
		}

		if (is_alert() && Enemy::get_collider().grounded()) {
			if (cooldowns.post_punch.is_complete()) {
				request(MinigusState::run);
				if (attacks.punch.sensor.active()) { request(MinigusState::punch); }
			} else {
				request(MinigusState::jumpsquat);
			}
		}

		if (Enemy::health_indicator.get_amount() < -80 && flags.state.test(StateFlags::vulnerable)) { request(MinigusState::drink); }
		if (cooldowns.vulnerability.is_complete() && flags.state.test(StateFlags::vulnerable)) { request(MinigusState::drink); }
	}

	if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { request(MinigusState::turn); }
	movement_direction.lnr = Enemy::get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;

	if (!battle_mode()) { request(MinigusState::idle); }
	if (!get_collider().grounded()) { request(MinigusState::airborne); }

	if (!has_flag_set(BossFlags::second_phase) && half_health()) { set_flag(BossFlags::second_phase); }
	if (is_staggered()) { request(MinigusState::stagger); }

	if (half_health() && secondary_collider) {
		auto pos = get_secondary_collider().physics.position + random::random_vector_float(10.f, 40.f);
		if (svc.ticker.every_x_ticks(10) && random::percent_chance(5)) { map.effects.push_back(entity::Effect(svc, "puff", pos, {0.f, -0.01f}, 3)); }
	}

	if (player.is_dead()) { request(MinigusState::laugh); }

	console_complete = !m_context->console.has_value();

	if (Boss::consume_flag(BossFlags::start_battle)) {
		m_mode = MinigusMode::battle_one;
		svc.quest_table.progress_quest("minigus_dialogue", 1, 1);
		svc.data.save_dialogue_quests();
		cooldowns.vulnerability.start();
	}

	/*if (status.test(MinigusFlags::goodbye)) { status.set(MinigusFlags::over_and_out); }
	if (status.test(MinigusFlags::over_and_out) && console_complete && is(MinigusState::exit)) {
		m_map->active_loot.push_back(item::Loot(svc, map, player, Enemy::get_collider().bounding_box.get_position(), {get_attributes().drop_range, get_attributes().loot_multiplier, 0, false, 0, 16}));
	}*/

	Minigus::state_function = Minigus::state_function();
}

void Minigus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);

	m_minigun.set_scale(p_animatable.get_scale());
	if (flags.state.test(StateFlags::invisible)) { return; }
	m_minigun.render(cam);
	sparkler.render(win, cam);

	if (debug::is_production()) {
		// win.draw(m_minigun);
		sprite_history.drag(win, cam);
	} else {
		// distant_range.render(win, cam);
	}
	if (is(MinigusState::punch)) { attacks.punch.render(win, cam); }
	attacks.rush.render(win, cam);
}

void Minigus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::gui_render(svc, win, cam);
	debug();
}

fsm::StateFunction Minigus::update_idle() {
	p_state.actual = MinigusState::idle;
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
		is_hostile() ? request(MinigusState::punch) : request(MinigusState::reload);
		if (change_state(MinigusState::reload, get_params("reload"))) { return MINIGUS_BIND(update_reload); }
		if (change_state(MinigusState::punch, get_params("punch"))) { return MINIGUS_BIND(update_punch); }
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
		p_services->camera_controller.shake(2, 0.02f, 100, 20);
	}
	get_collider().physics.acceleration.x = attributes.speed * directions.actual.as_float();
	if (p_animatable.animation.complete()) {
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_run);
}

fsm::StateFunction Minigus::update_shoot() {
	p_state.actual = MinigusState::shoot;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	shoot();
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		if (has_flag_set(BossFlags::second_phase)) {
			m_flags.set(MinigusFlags::jump_shoot);
			request(MinigusState::reload);
			if (change_state(MinigusState::reload, get_params("reload"))) { return MINIGUS_BIND(update_reload); }
		}
		if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		if (change_state(MinigusState::idle, get_params("idle"))) { return MINIGUS_BIND(update_idle); }
	}
	return MINIGUS_BIND(update_shoot);
}

fsm::StateFunction Minigus::update_jump_shoot() {
	p_state.actual = MinigusState::jump_shoot;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	shoot();
	if (p_animatable.animation.just_started()) { m_flags.reset(MinigusFlags::jump_shoot); }
	if (p_animatable.animation.complete()) {
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_jump_shoot);
}

fsm::StateFunction Minigus::update_jumpsquat() {
	p_state.actual = MinigusState::jumpsquat;
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (p_animatable.animation.complete()) {
		if (m_flags.test(MinigusFlags::jump_shoot)) {
			get_collider().physics.velocity.x = directions.actual.as_float() * -12.f;
			get_collider().physics.velocity.y = -20.f;
			p_services->soundboard.play_sound("minigus_woob", get_collider().get_center());
			request(MinigusState::jump_shoot);
			if (change_state(MinigusState::airborne, get_params("jump_shoot"))) { return MINIGUS_BIND(update_jump_shoot); }
		}
		get_collider().physics.velocity.x = directions.actual.as_float() * 8.f;
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
		auto const shockwave_speed = 0.8f;
		attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 0, {shockwave_speed * directions.actual.as_float(), 0.f}}});
		attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
		attacks.shockwaves.back().start();
		attacks.shockwaves.push_back(entity::Shockwave{{30, 600, 0, {-shockwave_speed * directions.actual.as_float(), 0.f}}});
		attacks.shockwaves.back().origin = Enemy::get_collider().get_bottom();
		attacks.shockwaves.back().start();
		m_map->spawn_effect(*p_services, "strong_slam", get_collider().get_center() + sf::Vector2f{0.f, 20.f});
	}
	if (p_animatable.animation.complete()) {
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
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
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
	if (p_animatable.animation.just_started()) { p_services->soundboard.play_sound("minigus_laugh_1", get_collider().get_center()); }
	if (p_animatable.animation.complete()) {
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
	if (change_state(MinigusState::turn, get_params("turn"))) { return MINIGUS_BIND(update_turn); }
	if (p_animatable.animation.just_started()) {}
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
	if (p_animatable.animation.complete()) {
		if (change_state(MinigusState::jump_shoot, get_params("jump_shoot"))) { return MINIGUS_BIND(update_jump_shoot); }
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
		request(MinigusState::charge);
		if (change_state(MinigusState::charge, get_params("charge"))) { return MINIGUS_BIND(update_charge); }
	}
	return MINIGUS_BIND(update_wind_up);
}

fsm::StateFunction Minigus::update_charge() {
	p_state.actual = MinigusState::charge;
	get_collider().physics.velocity.x = directions.actual.as_float() * 20.f;
	if (p_animatable.animation.just_started()) {}
	if (change_state(MinigusState::stagger, get_params("stagger"))) { return MINIGUS_BIND(update_stagger); }
	if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::laugh);
		if (change_state(MinigusState::laugh, get_params("laugh"))) { return MINIGUS_BIND(update_laugh); }
	}
	return MINIGUS_BIND(update_charge);
}

fsm::StateFunction Minigus::update_exit() {
	p_state.actual = MinigusState::exit;
	if (p_animatable.animation.just_started()) {}
	if (p_animatable.animation.complete()) {
		request(MinigusState::airborne);
		if (change_state(MinigusState::airborne, get_params("airborne"))) { return MINIGUS_BIND(update_airborne); }
	}
	return MINIGUS_BIND(update_exit);
}

void Minigus::debug() {
	static auto sz = ImVec2{180.f, 350.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Minigus Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("idle")) { request(MinigusState::idle); }
		if (ImGui::Button("run")) { request(MinigusState::run); }
		if (ImGui::Button("shoot")) { request(MinigusState::shoot); }
		if (ImGui::Button("jump_shoot")) { request(MinigusState::jump_shoot); }
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

void Minigun::render(sf::Vector2f const cam) { set_position(m_physics.position - cam + offset); }

} // namespace fornani::enemy
