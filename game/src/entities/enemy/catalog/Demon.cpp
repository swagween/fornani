#include "fornani/entities/enemy/catalog/Demon.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Demon::Demon(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "demon"), m_services(&svc), m_map(&map),
	  parts{.spear{svc.assets.get_texture("demon_spear"), 2.0f, 0.85f, {-16.f, 8.f}}, .sword{svc.assets.get_texture("demon_sword"), 2.0f, 0.85f, {-4.f, 8.f}}, .shield{svc.assets.get_texture("demon_shield"), 2.0f, 0.85f, {-28.f, 8.f}}},
	  m_variant{static_cast<DemonVariant>(variant)} {
	auto demon_framerate = 24;
	m_params = {{"idle", {0, 6, demon_framerate, -1}},	   {"turn", {9, 1, demon_framerate * 4, 0}}, {"run", {6, 4, demon_framerate, -1}},		{"jump", {7, 1, demon_framerate * 3, 0}},		{"signal", {10, 1, demon_framerate * 3, 2}},
				{"rush", {11, 1, demon_framerate * 2, 0}}, {"stab", {11, 3, demon_framerate, 0}},	 {"uppercut", {14, 3, demon_framerate, 0}}, {"jumpsquat", {11, 1, demon_framerate * 3, 0}}, {"dormant", {18, 1, demon_framerate * 5, -1}}};

	animation.set_params(get_params("dormant"));
	if (map.get_style_id() == 5) { animation.set_params(get_params("idle")); }
	get_collider().physics.maximum_velocity = {8.f, 12.f};
	get_collider().flags.general.set(shape::General::complex);
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_collider().physics.ground_friction = {0.97f, 0.97f};
	get_collider().physics.maximum_velocity.x = 36.f;
	get_secondary_collider().set_dimensions({39.f, 39.f});
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	attacks.stab.sensor.bounds.setRadius(10);
	attacks.stab.sensor.drawable.setFillColor(colors::blue);
	parts.shield.set_team(arms::Team::guardian);
	flags.state.set(StateFlags::no_shake);

	cooldowns.awaken.start();
}

void Demon::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	player_behind(player) ? m_flags.set(DemonFlags::player_behind) : m_flags.reset(DemonFlags::player_behind);

	cooldowns.rush_hit.update();
	cooldowns.post_jump.update();
	cooldowns.post_rush.update();
	cooldowns.stab.update();

	attacks.stab.update();
	attacks.stab.handle_player(player);
	attacks.stab.disable();
	if (is_state(DemonState::stab)) {
		attacks.stab.enable();
		if (Enemy::animation.get_frame_count() != 1) { attacks.stab.disable(); }
	}
	if (is_state(DemonState::uppercut)) {
		attacks.stab.enable();
		if (Enemy::animation.get_frame_count() != 1) { attacks.stab.disable(); }
	}
	if (attacks.stab.hit.active()) {
		if (attacks.stab.hit.within_bounds(player.get_collider().bounding_box)) {
			if (!player.invincible()) { player.accumulated_forces.push_back({Enemy::directions.actual.as_float() * 4.f, -2.f}); }
			player.hurt();
		}
		for (auto& proj : map.active_projectiles) {
			if (proj.get_team() == arms::Team::skycorps) { continue; }
			if (attacks.stab.hit.within_bounds(proj.get_collider())) {
				map.effects.push_back(entity::Effect(svc, "inv_hit", proj.get_position()));
				random::percent_chance(50) ? svc.soundboard.flags.lynx.set(audio::Lynx::ping_1) : svc.soundboard.flags.lynx.set(audio::Lynx::ping_2);
				proj.destroy(false);
				svc.ticker.freeze_frame(3);
			}
		}
	}

	flags.state.set(StateFlags::vulnerable); // demon is always vulnerable
	caution.avoid_ledges(map, get_collider(), directions.actual, 3);

	// reset animation states to determine next animation state
	directions.desired.lnr = (player.get_collider().get_center().x < get_collider().get_center().x) ? LNR::left : LNR::right;
	directions.movement.lnr = get_collider().physics.velocity.x > 0.f ? LNR::right : LNR::left;
	if (!is_dormant()) {
		parts.spear.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
		parts.sword.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
		parts.shield.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
	}
	if (m_variant == DemonVariant::spearman) { parts.spear.set_hitbox(); }
	if (m_variant == DemonVariant::warrior) {
		parts.sword.set_hitbox();
		parts.shield.set_shield();
	}

	if (secondary_collider) {
		get_secondary_collider().physics.position = get_collider().physics.position - sf::Vector2f{0.f, 20.f};
		get_secondary_collider().physics.position.x += 1.f;
		get_secondary_collider().sync_components();
		if (player.get_collider().hurtbox.overlaps(get_secondary_collider().bounding_box) && !is_dormant()) { player.hurt(); }
	}

	if (svc.ticker.every_x_ticks(200)) {
		if (random::percent_chance(4) && !caution.danger()) { request(DemonState::run); }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.demon.set(audio::Demon::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	if (is_hostile() && !cooldowns.post_rush.running()) { request(DemonState::signal); }
	if (is_hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) { request(DemonState::jumpsquat); } // player is already in hostile range

	if (is_alert() && !is_hostile() && svc.ticker.every_x_ticks(32)) {
		if (random::percent_chance(50)) {
			request(DemonState::run);
		} else {
			request(DemonState::jumpsquat);
		}
	}

	if (!is_hostile() && !is_alert()) {
		if (svc.ticker.every_x_ticks(48)) {
			if (random::percent_chance(10)) { request(DemonState::run); }
		}
	}
	auto incoming_projectile = caution.projectile_detected(map, Enemy::physical.hostile_range, arms::Team::guardian);
	if (incoming_projectile.lnr != LNR::neutral) {
		if (incoming_projectile.lnr != Enemy::directions.actual.lnr) {
			if (m_variant == DemonVariant::duelist) {
				random::percent_chance(50) ? request(DemonState::stab) : request(DemonState::uppercut);
			} else {
				request(DemonState::jump);
			}
			m_flags.set(DemonFlags::parrying);
		}
	}

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lnr != directions.desired.lnr) { request(DemonState::turn); }

	state_function = state_function();
}

void Demon::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died() || is_state(DemonState::dormant)) { return; }
	if (m_variant == DemonVariant::spearman) {
		parts.spear.render(svc, win, cam);
	} else if (m_variant == DemonVariant::warrior) {
		parts.shield.render(svc, win, cam);
		if (!is_state(DemonState::stab) && !is_state(DemonState::uppercut)) { parts.sword.render(svc, win, cam); }
	} else {
		if (!is_state(DemonState::stab) && !is_state(DemonState::uppercut)) { parts.sword.render(svc, win, cam); }
	}
	if (svc.greyblock_mode()) {
		if (attacks.stab.hit.active()) { attacks.stab.render(win, cam); }
	}
}

fsm::StateFunction Demon::update_idle() {
	m_state.actual = DemonState::idle;
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(DemonState::turn, get_params("turn"))) { return DEMON_BIND(update_turn); }
	if (change_state(DemonState::stab, get_params("stab"))) { return DEMON_BIND(update_stab); }
	if (change_state(DemonState::uppercut, get_params("uppercut"))) { return DEMON_BIND(update_uppercut); }
	if (change_state(DemonState::signal, get_params("signal"))) { return DEMON_BIND(update_signal); }
	if (change_state(DemonState::run, get_params("run"))) { return DEMON_BIND(update_run); }
	if (change_state(DemonState::jumpsquat, get_params("jumpsquat"))) { return DEMON_BIND(update_jumpsquat); }
	return DEMON_BIND(update_idle);
};

fsm::StateFunction Demon::update_turn() {
	m_state.actual = DemonState::turn;
	if (animation.complete()) {
		request_flip();
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	return DEMON_BIND(update_turn);
};

fsm::StateFunction Demon::update_run() {
	m_state.actual = DemonState::run;
	auto facing = directions.actual.lnr == LNR::left ? -1.f : 1.f;
	get_collider().physics.apply_force({attributes.speed * facing, 0.f});
	if (change_state(DemonState::turn, get_params("turn"))) { return DEMON_BIND(update_turn); }
	if (change_state(DemonState::stab, get_params("stab"))) { return DEMON_BIND(update_stab); }
	if (change_state(DemonState::uppercut, get_params("uppercut"))) { return DEMON_BIND(update_uppercut); }
	if (caution.danger() || animation.complete()) {
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	if (change_state(DemonState::turn, get_params("turn"))) { return DEMON_BIND(update_turn); }
	if (change_state(DemonState::signal, get_params("signal"))) { return DEMON_BIND(update_signal); }
	return DEMON_BIND(update_run);
}

fsm::StateFunction Demon::update_jump() {
	m_state.actual = DemonState::jump;
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = random::percent_chance(50) ? -1.f : 1.f;
		if (cooldowns.post_rush.running()) { rand_jump = directions.actual.lnr == LNR::left ? 1.f : -1.f; } // always jump backwards after a rush otherwise it feels unfair
	}
	if (cooldowns.jump.running()) { get_collider().physics.apply_force({0, -2.5f}); }
	if (!get_collider().grounded()) { get_collider().physics.apply_force({rand_jump * 2.f, 0.f}); }
	cooldowns.jump.update();
	if (cooldowns.jump.is_complete() && get_collider().grounded()) {
		cooldowns.post_jump.start();
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	return DEMON_BIND(update_jump);
}

fsm::StateFunction Demon::update_signal() {
	m_state.actual = DemonState::signal;
	if (animation.just_started()) { m_services->soundboard.flags.demon.set(audio::Demon::up_snort); }
	shake();
	if (animation.complete()) {
		switch (m_variant) {
		case DemonVariant::spearman:
			request(DemonState::rush);
			if (change_state(DemonState::rush, get_params("rush"))) { return DEMON_BIND(update_rush); }
			break;
		case DemonVariant::warrior:
			request(DemonState::stab);
			if (change_state(DemonState::stab, get_params("stab"))) { return DEMON_BIND(update_stab); }
			break;
		case DemonVariant::duelist:
			request(DemonState::stab);
			if (change_state(DemonState::stab, get_params("stab"))) { return DEMON_BIND(update_stab); }
			break;
		}
	}
	return DEMON_BIND(update_signal);
}

fsm::StateFunction Demon::update_rush() {
	m_state.actual = DemonState::rush;
	if (caution.danger()) {
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	auto force = 34.f * directions.actual.as_float();
	if (animation.just_started()) { get_collider().physics.apply_force({force, 0.f}); }
	parts.spear.move({directions.actual.as_float() * 70.f, 0.f});
	if (animation.complete()) {
		cooldowns.post_rush.start();
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	return DEMON_BIND(update_rush);
}

fsm::StateFunction Demon::update_stab() {
	m_state.actual = DemonState::stab;
	if (animation.just_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
		cooldowns.stab.start();
	}
	if (cooldowns.stab.running()) {
		auto force = 34.f * directions.actual.as_float();
		get_collider().physics.acceleration.x = force;
	}
	attacks.stab.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 52.f, 4.f});
	attacks.stab.set_constant_radius(32.f);
	if (animation.is_complete()) {
		m_flags.reset(DemonFlags::parrying);
		cooldowns.post_rush.start();
		if (!m_flags.test(DemonFlags::player_behind)) {
			request(DemonState::uppercut);
			if (change_state(DemonState::uppercut, get_params("uppercut"))) { return DEMON_BIND(update_uppercut); }
		}
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	return DEMON_BIND(update_stab);
}

fsm::StateFunction Demon::update_uppercut() {
	m_state.actual = DemonState::uppercut;
	if (animation.just_started()) {
		random::percent_chance(50) ? m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_1) : m_services->soundboard.flags.lynx.set(audio::Lynx::swipe_2);
		cooldowns.stab.start();
	}
	attacks.stab.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 22.f, -28.f});
	attacks.stab.set_constant_radius(48.f);
	if (animation.is_complete()) {
		m_flags.reset(DemonFlags::parrying);
		cooldowns.post_rush.start();
		request(DemonState::idle);
		if (change_state(DemonState::idle, get_params("idle"))) { return DEMON_BIND(update_idle); }
	}
	return DEMON_BIND(update_uppercut);
}

fsm::StateFunction Demon::update_dormant() {
	m_state.actual = DemonState::dormant;
	flags.state.reset(StateFlags::vulnerable);
	is_hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable_move);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		flags.state.set(StateFlags::vulnerable);
		m_map->effects.push_back(entity::Effect(*m_services, "small_explosion", get_collider().get_center(), {}, 2));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		m_services->soundboard.flags.demon.set(audio::Demon::snort);
		request(DemonState::jump);
		if (change_state(DemonState::jump, get_params("jump"))) { return DEMON_BIND(update_jump); }
	}
	return DEMON_BIND(update_dormant);
}

fsm::StateFunction Demon::update_jumpsquat() {
	m_state.actual = DemonState::jumpsquat;
	if (animation.just_started()) { m_services->soundboard.flags.demon.set(audio::Demon::snort); }
	if (animation.complete()) {
		request(DemonState::jump);
		if (change_state(DemonState::jump, get_params("jump"))) { return DEMON_BIND(update_jump); }
	}
	return DEMON_BIND(update_jumpsquat);
}

bool Demon::change_state(DemonState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
