
#include "fornani/entities/enemy/boss/Minigus.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

static bool b_start{};
static void start_battle(int battle) { b_start = true; }

Minigus::Minigus(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console)
	: Enemy(svc, map, "minigus"), gun(svc, "minigun"), soda(svc, "soda_gun"), m_services(&svc), NPC(svc, map, std::string_view{"minigus"}), m_map(&map), health_bar(svc, "minigus"),
	  sparkler(svc, Enemy::get_collider().get_vicinity_rect().size, colors::ui_white, "minigus"), m_console{&console}, m_mode{MinigusMode::neutral}, m_minigun{svc},
	  attacks{.left_shockwave{{50, 600, 3, {-0.6f, 0.f}}}, .right_shockwave{{50, 600, 3, {0.6f, 0.f}}}} {

	svc.events.register_event(std::make_unique<Event<int>>("StartBattle", &start_battle));

	Enemy::animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().set_team(arms::Team::skycorps);
	soda.get().set_team(arms::Team::skycorps);
	gun.cycle.set_order(3);
	Enemy::get_collider().physics.maximum_velocity = {8.f, 18.f};
	Enemy::get_collider().physics.set_constant_friction({0.97f, 0.989f});
	cooldowns.vulnerability.start();
	cooldowns.exit.start();
	afterlife = 2000;

	set_force_interact(true);
	flags.general.set(GeneralFlags::post_death_render);
	flags.general.set(GeneralFlags::has_invincible_channel);

	get_secondary_collider().set_dimensions({48.f, 36.f});
	m_minigun.center();
	m_minigun.set_parameters(m_minigun.neutral);
	flags.state.set(StateFlags::vulnerable);
	flags.state.set(StateFlags::no_slowdown);

	attacks.punch.sensor.bounds.setRadius(60);
	attacks.punch.sensor.drawable.setFillColor(colors::blue);
	attacks.punch.hit.bounds.setRadius(28);
	attacks.punch.origin = {-10.f, -26.f};

	attacks.uppercut.sensor.bounds.setRadius(60);
	attacks.uppercut.sensor.drawable.setFillColor(colors::blue);
	attacks.uppercut.hit.bounds.setRadius(28);
	attacks.uppercut.origin = {-8.f, 36.f};

	attacks.rush.sensor.bounds.setRadius(60);
	attacks.rush.hit.bounds.setRadius(40);
	attacks.rush.origin = {40.f, 10.f};
	attacks.rush.hit_offset = {-20.f, 0.f};

	distant_range.set_dimensions({720, 800});
	Enemy::get_collider().stats.GRAV = 6.0f;
	sprite_direction.lnr = LNR::left;
	Enemy::directions.actual.lnr = LNR::left;

	auto prog = svc.quest_table.get_quest_progression("minigus_dialogue");
	auto which = prog == 0 ? 1 : 4;
	push_conversation(which);

	sparkler.set_dimensions(Enemy::get_collider().get_vicinity_rect().size);
}

void Minigus::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (init) {
		m_minigun.set_physics_position(Enemy::get_collider().get_center());
		init = false;
	}
	sparkler.update(svc);
	sparkler.set_position(Enemy::get_collider().get_vicinity_rect().position);
	health_bar.update(health.get_normalized());

	if (map.off_the_bottom(Enemy::get_collider().physics.position)) {
		post_death.cancel();
		flags.general.set(GeneralFlags::no_loot);
	}

	if (Enemy::directions.actual.lnr == LNR::left) {
		attacks.punch.set_position(Enemy::get_collider().physics.position);
		attacks.uppercut.set_position(Enemy::get_collider().physics.position);
		attacks.rush.set_position(Enemy::get_collider().physics.position);
		attacks.punch.origin.x = -10.f;
		attacks.uppercut.origin.x = -8.f;
		attacks.rush.origin.x = 40.f;
		attacks.rush.hit_offset.x = -20.f;
	} else {
		sf::Vector2f dir_offset{Enemy::get_collider().bounding_box.get_dimensions().x, 0.f};
		attacks.punch.set_position(Enemy::get_collider().physics.position + dir_offset);
		attacks.uppercut.set_position(Enemy::get_collider().physics.position + dir_offset);
		attacks.rush.set_position(Enemy::get_collider().physics.position + dir_offset);
		attacks.punch.origin.x = 10.f;
		attacks.uppercut.origin.x = 8.f;
		attacks.rush.origin.x = -40.f;
		attacks.rush.hit_offset.x = 20.f;
	}

	gun.update(svc, map, *this);
	soda.update(svc, map, *this);
	cooldowns.firing.update();
	cooldowns.post_charge.update();
	cooldowns.post_punch.update();
	cooldowns.hurt.update();
	cooldowns.player_punch.update();
	if (status.test(MinigusFlags::exit_scene)) { cooldowns.exit.update(); }
	if (was_introduced()) { cooldowns.vulnerability.update(); }

	if (svc.ticker.every_x_ticks(32)) { hurt_color.update(); }

	attacks.punch.update();
	attacks.uppercut.update();
	attacks.rush.update();
	attacks.punch.handle_player(player);
	attacks.uppercut.handle_player(player);
	attacks.rush.handle_player(player);

	attacks.left_shockwave.origin = Enemy::get_collider().physics.position + sf::Vector2f{0.f, Enemy::get_collider().bounding_box.get_dimensions().y};
	attacks.right_shockwave.origin = Enemy::get_collider().physics.position + Enemy::get_collider().bounding_box.get_dimensions();
	attacks.left_shockwave.update(svc, map);
	attacks.right_shockwave.update(svc, map);

	attacks.left_shockwave.handle_player(player);
	attacks.right_shockwave.handle_player(player);

	if (status.test(MinigusFlags::battle_mode)) {
		if (attacks.left_shockwave.hit.active() && !cooldowns.player_punch.running()) {
			player.hurt(1);
			if (!player.invincible()) { player.accumulated_forces.push_back({-40.f, -4.f}); }
			attacks.left_shockwave.hit.deactivate();
			cooldowns.player_punch.start();
		}
		if (attacks.right_shockwave.hit.active() && !cooldowns.player_punch.running()) {
			player.hurt(1);
			if (!player.invincible()) { player.accumulated_forces.push_back({40.f, -4.f}); }
			attacks.right_shockwave.hit.deactivate();
			cooldowns.player_punch.start();
		}
		if (Enemy::animation.get_frame() == 30 && attacks.punch.hit.active() && !cooldowns.player_punch.running()) {
			player.hurt(1);
			auto sign = Enemy::directions.actual.lnr == LNR::left ? -1.f : 1.f;
			player.accumulated_forces.push_back({sign * 10.f, -4.f});
			attacks.punch.sensor.deactivate();
			cooldowns.player_punch.start();
		}
		if (Enemy::animation.get_frame() == 37 && attacks.uppercut.hit.active() && !cooldowns.player_punch.running()) {
			player.hurt(1);
			auto sign = Enemy::directions.actual.lnr == LNR::left ? -1.f : 1.f;
			player.accumulated_forces.push_back({sign * 10.f, -4.f});
			attacks.uppercut.sensor.deactivate();
			cooldowns.player_punch.start();
		}
		if (is(MinigusState::rush) && attacks.rush.sensor.active() && !cooldowns.player_punch.running()) {
			auto sign = Enemy::directions.actual.lnr == LNR::left ? -1.f : 1.f;
			if ((sign == -1.f && Enemy::player_behind(player)) || (sign == 1.f && !Enemy::player_behind(player))) {
				player.hurt(1);
				player.accumulated_forces.push_back({sign * 10.f, -4.f});
				attacks.rush.sensor.deactivate();
				cooldowns.player_punch.start();
			}
		}
	}
	for (auto& breakable : map.breakables) {
		if (Enemy::get_collider().jumpbox.overlaps(breakable->get_bounding_box())) { breakable->on_smash(svc, map, 4); }
		if (is(MinigusState::rush) && attacks.rush.hit.within_bounds(breakable->get_bounding_box())) {
			if (svc.ticker.every_x_ticks(18)) { breakable->on_smash(svc, map, 1); }
		}
	}

	m_minigun.update(Enemy::get_collider().get_center());
	m_minigun.offset = Enemy::directions.actual.left() ? sf::Vector2f{-32.f, 0.f} : sf::Vector2f{32.f, 0.f};
	auto gun_base = Enemy::get_collider().physics.position + Enemy::get_collider().dimensions * 0.5f;
	auto gun_point = Enemy::directions.actual.lnr == LNR::left ? gun_base - sf::Vector2f{m_minigun.get_f_dimensions().x * 2.f, -6.f} : gun_base + sf::Vector2f{m_minigun.get_f_dimensions().x * 2.f, 6.f};
	gun.get().set_barrel_point(gun_point);
	gun_point.y -= 64;
	soda.get().set_barrel_point(gun_point);

	Enemy::directions.desired.lnr = Enemy::player_behind(player) ? LNR::left : LNR::right;
	Enemy::update(svc, map, player);

	if (secondary_collider) {
		get_secondary_collider().physics.position = Enemy::get_collider().physics.position;
		get_secondary_collider().physics.position.y -= get_secondary_collider().dimensions.y;
		get_secondary_collider().physics.position.x += Enemy::directions.actual.lnr == LNR::left ? 0 : Enemy::get_collider().dimensions.x - get_secondary_collider().dimensions.x;
		get_secondary_collider().sync_components();
		if (status.test(MinigusFlags::battle_mode) && player_collision()) { player.get_collider().handle_collider_collision(get_secondary_collider()); }
	}
	distant_range.set_position(Enemy::get_collider().bounding_box.get_position() - (distant_range.get_dimensions() * 0.5f) + (Enemy::get_collider().dimensions * 0.5f));
	player.get_collider().bounding_box.overlaps(distant_range) ? status.set(MinigusFlags::distant_range_activated) : status.reset(MinigusFlags::distant_range_activated);
	player.on_crush(map);

	// state management

	if (status.test(MinigusFlags::distant_range_activated) && !is_alert() && !is_hostile()) { request(MinigusState::run); }
	if (attacks.uppercut.sensor.active() && cooldowns.post_punch.is_complete()) { request(MinigusState::uppercut); }
	if (attacks.punch.sensor.active() && cooldowns.post_punch.is_complete()) { request(MinigusState::punch); }

	if (flags.state.test(StateFlags::hurt)) {
		cooldowns.hurt.start();
		if (random::percent_chance(40)) {
			m_services->soundboard.flags.minigus.set(audio::Minigus::hurt_1);
		} else if (random::percent_chance(40)) {
			m_services->soundboard.flags.minigus.set(audio::Minigus::hurt_2);
		} else {
			m_services->soundboard.flags.minigus.set(audio::Minigus::hurt_3);
		}
		flags.state.reset(StateFlags::hurt);
	}

	if (just_died()) {}

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
			if (attacks.uppercut.sensor.active()) { request(MinigusState::uppercut); }
			if (attacks.punch.sensor.active()) { request(MinigusState::punch); }
		} else {
			request(MinigusState::jumpsquat);
		}
	}

	if (Enemy::health_indicator.get_amount() < -80 && flags.state.test(StateFlags::vulnerable)) { request(MinigusState::drink); }
	if (cooldowns.vulnerability.is_complete() && flags.state.test(StateFlags::vulnerable)) { request(MinigusState::drink); }

	if (Enemy::directions.actual.lnr != Enemy::directions.desired.lnr) { request(MinigusState::turn); }
	movement_direction.lnr = Enemy::get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;

	if (!status.test(MinigusFlags::battle_mode)) { request(MinigusState::idle); }

	if (!status.test(MinigusFlags::second_phase) && half_health()) { request(MinigusState::struggle); }

	if (half_health() && secondary_collider) {
		auto pos = get_secondary_collider().physics.position + random::random_vector_float(10.f, 40.f);
		if (svc.ticker.every_x_ticks(10) && random::percent_chance(5)) { map.effects.push_back(entity::Effect(svc, "puff", pos, {0.f, -0.01f}, 3)); }
	}

	if (player.is_dead()) { request(MinigusState::laugh); }

	// NPC stuff
	if (player.get_collider().bounding_box.overlaps(distant_range) && !was_introduced() && is_force_interact()) { set_distant_interact(true); }

	NPC::update(svc, map, *m_console, player);
	if (m_console) {
		if (m_console->has_value()) {
			m_console->value()->set_no_exit(true);
			set_force_interact(false);
		}
	}
	console_complete = static_cast<bool>(m_console);

	if (b_start) {
		m_mode = MinigusMode::battle_one;
		status.set(MinigusFlags::battle_mode);
		svc.quest_table.progress_quest("minigus_dialogue", 1, 1);
		svc.data.save_quests();
		set_distant_interact(false);
		set_force_interact(false);
		svc.music_player.load(svc.finder, "scuffle");
		svc.music_player.play_looped();
		cooldowns.vulnerability.start();
		b_start = false;
	}

	if (was_introduced() && !status.test(MinigusFlags::theme_song)) {
		svc.music_player.load(svc.finder, "minigus");
		svc.music_player.play_looped();
		status.set(MinigusFlags::theme_song);
	}

	if (health.is_dead() && !status.test(MinigusFlags::over_and_out) && !status.test(MinigusFlags::goodbye)) { request(MinigusState::struggle); }
	if (status.test(MinigusFlags::goodbye)) { status.set(MinigusFlags::over_and_out); }
	if (status.test(MinigusFlags::over_and_out) && console_complete && m_state.actual == MinigusState::exit) {
		m_map->active_loot.push_back(item::Loot(svc, map, player, get_attributes().drop_range, get_attributes().loot_multiplier, Enemy::get_collider().bounding_box.get_position()));
	}

	Minigus::state_function = Minigus::state_function();
}

void Minigus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	NPC::render(win, cam);
	Enemy::render(svc, win, cam);

	m_minigun.set_scale(Enemy::get_scale());
	m_minigun.render(cam);
	sparkler.render(win, cam);

	if (!svc.greyblock_mode()) {
		win.draw(m_minigun);
		sprite_history.drag(win, cam);
	} else {
		if (is(MinigusState::punch)) { attacks.punch.render(win, cam); }
		if (is(MinigusState::uppercut)) { attacks.uppercut.render(win, cam); }
		attacks.uppercut.render(win, cam);
		attacks.rush.render(win, cam);
		attacks.left_shockwave.render(win, cam);
		attacks.right_shockwave.render(win, cam);
	}
}

void Minigus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (status.test(MinigusFlags::battle_mode)) { health_bar.render(win); }
}

fsm::StateFunction Minigus::update_idle() {
	m_state.actual = MinigusState::idle;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "idle"); }
	if (!is_battle_mode()) { request(MinigusState::idle); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (change_state(MinigusState::laugh, laugh)) { return MINIGUS_BIND(update_laugh); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (change_state(MinigusState::jump_shoot, jump_shoot)) { return MINIGUS_BIND(update_jump_shoot); }
	if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
	if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
	if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
	if (change_state(MinigusState::drink, drink)) { return MINIGUS_BIND(update_drink); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	return MINIGUS_BIND(update_idle);
}

fsm::StateFunction Minigus::update_shoot() {
	m_state.actual = MinigusState::shoot;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "shoot"); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.just_started()) {
		m_services->soundboard.flags.minigus.set(audio::Minigus::doge);
		m_minigun.set_parameters(m_minigun.charging);
		m_minigun.flags.set(MinigunFlags::charging);
		m_services->soundboard.flags.minigus.set(audio::Minigus::charge);
	}
	if (!gun.get().cooling_down() && !m_minigun.flags.test(MinigunFlags::charging)) {
		gun.cycle.update();
		gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2f{0.f, 10.f} : (gun.cycle.get_alternator() % 2 == 1 ? sf::Vector2f{0.f, 20.f} : sf::Vector2f{0.f, 15.f});
		gun.shoot(*m_services, *m_map);
		m_map->spawn_projectile_at(*m_services, gun.get(), gun.get().get_barrel_point());
		m_map->shake_camera();
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (m_minigun.flags.test(MinigunFlags::charging)) {
		if (m_minigun.animation.complete()) {
			m_minigun.flags.reset(MinigunFlags::charging);
			m_minigun.set_parameters(m_minigun.firing);
			cooldowns.post_charge.start();
		}
	}
	if (!m_minigun.flags.test(MinigunFlags::charging)) {
		if (m_minigun.animation.complete()) {
			m_minigun.flags.set(MinigunFlags::exhausted);
			m_minigun.set_parameters(m_minigun.deactivated);
			counters.snap.cancel();
			cooldowns.firing.start();

			if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
			if (invincible()) {
				request(MinigusState::rush);
				Enemy::animation.set_params(rush);
				return MINIGUS_BIND(update_rush);
			}
			if (random::percent_chance(50)) {
				request(MinigusState::run);
				Enemy::animation.set_params(run);
				return MINIGUS_BIND(update_run);
			} else {
				request(MinigusState::laugh);
				Enemy::animation.set_params(laugh);
				return MINIGUS_BIND(update_laugh);
			}
		}
	}
	return MINIGUS_BIND(update_shoot);
}

fsm::StateFunction Minigus::update_jumpsquat() {
	m_state.actual = MinigusState::jumpsquat;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "jumpsquat"); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.complete()) {
		request(MinigusState::jump);
		Enemy::animation.set_params(jump);
		return MINIGUS_BIND(update_jump);
	}
	return MINIGUS_BIND(update_jumpsquat);
}

fsm::StateFunction Minigus::update_hurt() {
	m_state.actual = MinigusState::hurt;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "hurt"); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (Enemy::animation.complete()) {
		flags.state.reset(StateFlags::hurt);
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_hurt);
}

fsm::StateFunction Minigus::update_jump() {
	m_state.actual = MinigusState::jump;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "jump"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::woob); }
	cooldowns.jump.update();
	if (Enemy::animation.just_started()) { cooldowns.jump.start(); }
	auto sign = Enemy::directions.actual.lnr == LNR::left ? -1.f : 1.f;
	if (status.test(MinigusFlags::over_and_out)) { sign = 0; }
	if (cooldowns.jump.running()) { Enemy::get_collider().physics.apply_force({sign * 36.f, -8.f}); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (cooldowns.jump.is_complete() && status.test(MinigusFlags::over_and_out)) { flags.general.reset(GeneralFlags::map_collision); }
	if (Enemy::get_collider().grounded() && cooldowns.jump.is_complete()) {
		m_services->soundboard.flags.minigus.set(audio::Minigus::crash);
		m_services->soundboard.flags.minigus.set(audio::Minigus::land);
		m_services->camera_controller.shake(10, 0.3f, 200, 20);
		attacks.left_shockwave.start();
		attacks.right_shockwave.start();
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_jump);
}

fsm::StateFunction Minigus::update_jump_shoot() {
	m_state.actual = MinigusState::jump_shoot;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "jump_shoot"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::getit); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (cooldowns.pre_jump.get() != -1) { cooldowns.pre_jump.update(); }
	cooldowns.jump.update();
	auto sign = Enemy::directions.actual.lnr == LNR::left ? 1.f : -2.f;
	if (cooldowns.jump.running()) { Enemy::get_collider().physics.apply_force({sign * 4.f, -8.f}); }
	if (Enemy::animation.just_started()) {
		cooldowns.pre_jump.start();
		m_minigun.set_parameters(m_minigun.charging);
		m_minigun.flags.set(MinigunFlags::charging);
		m_services->soundboard.flags.minigus.set(audio::Minigus::charge);
	}
	if (cooldowns.pre_jump.is_complete() && !cooldowns.jump.running()) {
		cooldowns.pre_jump.nullify();
		cooldowns.jump.start();
	}
	if (!gun.get().cooling_down() && !m_minigun.flags.test(MinigunFlags::charging)) {
		gun.cycle.update();
		gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2f{0.f, 10.f} : (gun.cycle.get_alternator() % 2 == 1 ? sf::Vector2f{0.f, 20.f} : sf::Vector2f{0.f, 15.f});
		gun.shoot(*m_services, *m_map);
		m_map->shake_camera();
		m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
	}
	if (m_minigun.animation.complete() && m_minigun.flags.test(MinigunFlags::charging)) {
		m_minigun.flags.reset(MinigunFlags::charging);
		m_minigun.set_parameters(m_minigun.firing);
		cooldowns.post_charge.start();
	}
	if (m_minigun.animation.complete() && !m_minigun.flags.test(MinigunFlags::charging)) {
		m_minigun.flags.set(MinigunFlags::exhausted);
		m_minigun.set_parameters(m_minigun.deactivated);
		counters.snap.cancel();
		Enemy::set_channel(EnemyChannel::standard);
		cooldowns.firing.start();

		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }

		if (invincible()) {
			request(MinigusState::rush);
			Enemy::animation.set_params(rush);
			return MINIGUS_BIND(update_rush);
		}

		if (random::percent_chance(50)) {
			request(MinigusState::idle);
			Enemy::animation.set_params(idle);
			return MINIGUS_BIND(update_idle);
		} else {
			request(MinigusState::laugh);
			Enemy::animation.set_params(laugh);
			return MINIGUS_BIND(update_laugh);
		}
	}
	request(MinigusState::jump_shoot);
	return MINIGUS_BIND(update_jump_shoot);
}

fsm::StateFunction Minigus::update_reload() {
	m_state.actual = MinigusState::reload;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "reload"); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.complete()) {
		m_minigun.flags.reset(MinigunFlags::exhausted);
		m_minigun.set_parameters(m_minigun.neutral);
		m_services->soundboard.flags.minigus.set(audio::Minigus::deepspeak);
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }

		if (invincible()) {
			if (random::percent_chance(50)) {
				request(MinigusState::shoot);
				Enemy::animation.set_params(shoot);
				return MINIGUS_BIND(update_shoot);
			}
			request(MinigusState::jump_shoot);
			Enemy::animation.set_params(jump_shoot);
			return MINIGUS_BIND(update_jump_shoot);

		} else {
			if (random::percent_chance(40)) {
				request(MinigusState::laugh);
				Enemy::animation.set_params(laugh);
				return MINIGUS_BIND(update_laugh);
			}
			request(MinigusState::run);
			Enemy::animation.set_params(run);
			return MINIGUS_BIND(update_run);
		}
	}
	return MINIGUS_BIND(update_reload);
}

fsm::StateFunction Minigus::update_turn() {
	m_state.actual = MinigusState::turn;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "turn"); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	Enemy::directions.desired.lock();
	if (Enemy::animation.complete()) {
		Enemy::request_flip();
		if (invincible()) {
			counters.invincible_turn.update();
			if (m_minigun.flags.test(MinigunFlags::exhausted)) {
				request(MinigusState::reload);
				Enemy::animation.set_params(reload);
				return MINIGUS_BIND(update_reload);
			}
			if (counters.invincible_turn.get_count() > 3) {
				counters.invincible_turn.start();
				request(MinigusState::rush);
				Enemy::animation.set_params(rush);
				return MINIGUS_BIND(update_rush);
			}
			if (random::percent_chance(50)) {
				request(MinigusState::shoot);
				Enemy::animation.set_params(shoot);
				return MINIGUS_BIND(update_shoot);
			}
			request(MinigusState::jump_shoot);
			Enemy::animation.set_params(jump_shoot);
			return MINIGUS_BIND(update_jump_shoot);
		}
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
	return MINIGUS_BIND(update_turn);
}

fsm::StateFunction Minigus::update_run() {
	m_state.actual = MinigusState::run;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "run"); }
	auto sign = Enemy::directions.actual.lnr == LNR::left ? -1 : 1;
	Enemy::get_collider().physics.apply_force({Enemy::attributes.speed * sign, 0.f});
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
	if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	if (Enemy::animation.complete()) {
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
		if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
		if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_run);
}

fsm::StateFunction Minigus::update_punch() {
	m_state.actual = MinigusState::punch;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "punch"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::mother); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.get_frame() == 30 && !status.test(MinigusFlags::punched)) {
		m_map->effects.push_back(entity::Effect(*m_services, "small_flash", attacks.punch.hit.bounds.getPosition()));
		status.set(MinigusFlags::punched);
	}
	if (Enemy::animation.complete()) {
		status.reset(MinigusFlags::punched);
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_punch);
}

fsm::StateFunction Minigus::update_uppercut() {
	m_state.actual = MinigusState::uppercut;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "uppercut"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::momma); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.get_frame() == 37 && !status.test(MinigusFlags::punched)) {
		m_map->effects.push_back(entity::Effect(*m_services, "small_flash", attacks.uppercut.hit.bounds.getPosition()));
		status.set(MinigusFlags::punched);
	}
	if (Enemy::animation.complete()) {
		status.reset(MinigusFlags::punched);
		cooldowns.post_punch.start();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::rush, rush)) { return MINIGUS_BIND(update_rush); }
		if (change_state(MinigusState::snap, snap)) { return MINIGUS_BIND(update_snap); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_uppercut);
}

fsm::StateFunction Minigus::update_build_invincibility() {
	m_state.actual = MinigusState::build_invincibility;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "build_invincibility"); }
	if (Enemy::animation.just_started()) {
		m_services->soundboard.flags.minigus.set(audio::Minigus::grunt);
		m_services->soundboard.flags.minigus.set(audio::Minigus::build);
		sparkler.set_rate(6.f);
	}
	if (change_state(MinigusState::struggle, struggle)) {
		sparkler.set_rate(0.f);
		return MINIGUS_BIND(update_struggle);
	}
	cooldowns.hurt.cancel();
	if (Enemy::animation.complete()) {
		flags.state.reset(StateFlags::vulnerable);
		counters.snap.start();
		m_services->soundboard.flags.minigus.set(audio::Minigus::invincible);
		Animatable::set_channel(static_cast<int>(EnemyChannel::invincible));
		sparkler.set_rate(0.f);
		request(MinigusState::laugh);
		Enemy::animation.set_params(laugh);
		return MINIGUS_BIND(update_laugh);
	}
	return MINIGUS_BIND(update_build_invincibility);
}

fsm::StateFunction Minigus::update_laugh() {
	m_state.actual = MinigusState::laugh;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "laugh"); }
	if (Enemy::animation.just_started()) {
		if (random::percent_chance(50)) {
			m_services->soundboard.flags.minigus.set(audio::Minigus::laugh_1);
		} else {
			m_services->soundboard.flags.minigus.set(audio::Minigus::laugh_2);
		}
	}
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.complete()) {
		if (invincible()) {
			if (half_health() && counters.snap.get_count() < 2) {
				request(MinigusState::snap);
				Enemy::animation.set_params(snap);
				return MINIGUS_BIND(update_snap);
			}
			if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
			if (random::percent_chance(50)) {
				request(MinigusState::shoot);
				Enemy::animation.set_params(shoot);
				return MINIGUS_BIND(update_shoot);
			} else {
				request(MinigusState::jump_shoot);
				Enemy::animation.set_params(jump_shoot);
				return MINIGUS_BIND(update_jump_shoot);
			}
		}
		if (change_state(MinigusState::punch, punch)) { return MINIGUS_BIND(update_punch); }
		if (change_state(MinigusState::uppercut, uppercut)) { return MINIGUS_BIND(update_uppercut); }
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_laugh);
}

fsm::StateFunction Minigus::update_snap() {
	m_state.actual = MinigusState::snap;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "snap"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::snap); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.complete()) {
		for (int i{0}; i < 2; ++i) {
			auto randx = random::random_range_float(-80.f, 80.f);
			auto randy = random::random_range_float(-160.f, 0.f);
			sf::Vector2f rand_vec{randx, randy};
			sf::Vector2f spawn = Enemy::get_collider().get_center() + rand_vec;
			m_map->spawn_enemy(5, spawn);
		}
		counters.snap.update();
		request(MinigusState::laugh);
		Enemy::animation.set_params(laugh);
		return MINIGUS_BIND(update_laugh);
	}
	return MINIGUS_BIND(update_snap);
}

fsm::StateFunction Minigus::update_rush() {
	m_state.actual = MinigusState::rush;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "rush"); }
	cooldowns.rush.update();
	cooldowns.jump.update();
	flags.general.reset(GeneralFlags::player_collision);
	if (Enemy::animation.just_started()) { cooldowns.rush.start(); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	auto sign = Enemy::directions.actual.lnr == LNR::left ? -1.f : 1.f;
	Enemy::get_collider().physics.apply_force({sign * Enemy::attributes.speed * rush_speed, 0.f});
	if (cooldowns.rush.get() == 300 && half_health()) { cooldowns.jump.start(); }
	if (cooldowns.jump.running()) { Enemy::get_collider().physics.apply_force({0.f, -8.f}); }
	if (cooldowns.rush.is_complete()) {
		flags.general.set(GeneralFlags::player_collision);
		if (!flags.state.test(StateFlags::vulnerable)) { cooldowns.vulnerability.start(); }
		flags.state.set(StateFlags::vulnerable);
		request(MinigusState::idle);
		Enemy::animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	return MINIGUS_BIND(update_rush);
}

fsm::StateFunction Minigus::update_struggle() {
	m_state.actual = MinigusState::struggle;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "struggle"); }

	m_minigun.set_parameters(m_minigun.neutral);
	m_minigun.state = MinigunState::neutral;
	// always do
	if (secondary_collider) {
		sf::Vector2f pos = get_secondary_collider().physics.position + random::random_vector_float(0.f, 50.f);
		if (m_services->ticker.every_x_ticks(80)) { m_map->effects.push_back(entity::Effect(*m_services, "puff", pos, {}, 3)); }
	}
	Enemy::shake();

	// at half health
	if (half_health() && !status.test(MinigusFlags::second_phase)) {
		if (Enemy::animation.just_started()) {
			cooldowns.struggle.start();
			m_services->soundboard.flags.minigus.set(audio::Minigus::quick_breath);
		}
		cooldowns.struggle.update();
		if (cooldowns.struggle.is_complete()) {
			status.set(MinigusFlags::second_phase);
			stop_shaking();
			request(MinigusState::drink);
			Enemy::animation.set_params(drink);
			return MINIGUS_BIND(update_drink);
		}
	}

	// after health is empty
	if (health.is_dead()) {
		if (Enemy::animation.just_started()) {
			status.reset(MinigusFlags::battle_mode);
			set_distant_interact(true);
			set_force_interact(true);
			flush_conversations();
			push_conversation(2);
			m_services->music_player.pause();
			m_services->soundboard.flags.minigus.set(audio::Minigus::crash);
			m_services->soundboard.flags.minigus.set(audio::Minigus::quick_breath);
			m_services->soundboard.flags.minigus.set(audio::Minigus::long_moan);
		}
		if (!Enemy::animation.just_started() && !m_console->has_value() && !status.test(MinigusFlags::exit_scene)) {
			NANI_LOG_DEBUG(m_logger, "Exit cooldown started");
			status.set(MinigusFlags::exit_scene);
			cooldowns.exit.start();
		}
		post_death.start(afterlife);
		flags.state.set(StateFlags::special_death_mode);

		if (cooldowns.exit.is_complete() && status.test(MinigusFlags::exit_scene)) {
			NANI_LOG_DEBUG(m_logger, "Goodbye started");
			set_distant_interact(true);
			set_force_interact(true);
			m_services->music_player.stop();
			flush_conversations();
			push_conversation(3);
			status.reset(MinigusFlags::exit_scene);
			status.set(MinigusFlags::goodbye);
			stop_shaking();
			request(MinigusState::exit);
			Enemy::animation.set_params(idle);
			return MINIGUS_BIND(update_exit);
		}
	}

	request(MinigusState::struggle);
	return MINIGUS_BIND(update_struggle);
}

fsm::StateFunction Minigus::update_exit() {
	m_state.actual = MinigusState::exit;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "exit"); }
	if (status.test(MinigusFlags::over_and_out) && console_complete) {
		request(MinigusState::jumpsquat);
		Enemy::animation.set_params(jumpsquat);
		m_services->music_player.load(m_services->finder, "dusken");
		m_services->music_player.play_looped();
		return MINIGUS_BIND(update_jumpsquat);
	}
	return MINIGUS_BIND(update_exit);
}

fsm::StateFunction Minigus::update_drink() {
	m_state.actual = MinigusState::drink;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "drink"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::mother); }
	if (Enemy::animation.get_frame() == 48 && !status.test(MinigusFlags::soda_pop)) {
		m_services->soundboard.flags.minigus.set(audio::Minigus::soda);
		status.set(MinigusFlags::soda_pop);
	}
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.complete()) {
		m_services->soundboard.flags.minigus.set(audio::Minigus::poh);
		status.reset(MinigusFlags::soda_pop);
		request(MinigusState::throw_can);
		Enemy::animation.set_params(throw_can);
		return MINIGUS_BIND(update_throw_can);
	}
	return MINIGUS_BIND(update_drink);
}

fsm::StateFunction Minigus::update_throw_can() {
	m_state.actual = MinigusState::throw_can;
	if (Enemy::animation.just_started() && anim_debug) { NANI_LOG_DEBUG(m_logger, "throw can"); }
	if (Enemy::animation.just_started()) { m_services->soundboard.flags.minigus.set(audio::Minigus::pizza); }
	if (change_state(MinigusState::struggle, struggle)) { return MINIGUS_BIND(update_struggle); }
	if (Enemy::animation.get_frame() == 62 && !status.test(MinigusFlags::threw_can)) {
		m_map->spawn_projectile_at(*m_services, soda.get(), soda.get().get_barrel_point());
		status.set(MinigusFlags::threw_can);
	}
	if (Enemy::animation.complete()) {
		status.reset(MinigusFlags::threw_can);
		request(MinigusState::build_invincibility);
		Enemy::animation.set_params(build_invincibility);
		return MINIGUS_BIND(update_build_invincibility);
	}
	return MINIGUS_BIND(update_throw_can);
}

bool Minigus::change_state(MinigusState next, anim::Parameters params) {
	if (m_state.desired == next) {
		Enemy::animation.set_params(params);
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
