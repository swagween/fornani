#include "fornani/entities/enemy/catalog/Imp.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

constexpr static int imp_framerate{16};

Imp::Imp(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "imp"), m_services(&svc), m_map(&map),
	  parts{.weapon = random::percent_chance(50) ? entity::FloatingPart{svc.assets.get_texture("imp_knife"),
																		{36, 20},
																		{{0, 1, imp_framerate, -1}, {1, 1, imp_framerate, 0}, {2, 1, imp_framerate, -1}, {3, 3, imp_framerate, 0}, {5, 1, imp_framerate, 0}, {6, 1, imp_framerate, 0}},
																		{"idle", "lift", "run", "attack", "dormant", "swoosh"},
																		2.0f,
																		0.85f,
																		{0.f, -30.f},
																		1}
												 : entity::FloatingPart{svc.assets.get_texture("imp_fork"),
																		{41, 17},
																		{{0, 1, imp_framerate, -1}, {1, 1, imp_framerate, 0}, {2, 1, imp_framerate, -1}, {3, 3, imp_framerate, 9}, {0, 1, imp_framerate, -1}, {6, 1, imp_framerate, 0}},
																		{"idle", "lift", "run", "attack", "dormant", "swoosh"},
																		2.0f,
																		0.85f,
																		{0.f, -30.f},
																		2},
			.hand{colors::ui_black, {4.f, 4.f}, 2.0f, 0.85f, {-14.f, -4.f}}},
	  dormant{0, 1, imp_framerate, -1}, idle{1, 6, imp_framerate, -1}, turn{7, 3, imp_framerate, 0}, run{10, 8, imp_framerate, -1}, jump{18, 5, imp_framerate, 0}, fall{24, 3, imp_framerate, -1}, attack{27, 7, imp_framerate, 0},
	  m_variant{static_cast<ImpVariant>(variant)} {

	animation.set_params(dormant);
	get_collider().physics.maximum_velocity = {40.f, 12.f};
	get_collider().physics.air_friction = {0.95f, 0.999f};
	get_collider().flags.general.set(shape::General::complex);
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	attacks.stab.sensor.bounds.setRadius(10);
	attacks.stab.sensor.drawable.setFillColor(colors::blue);
	attacks.stab.hit.bounds.setRadius(28);
	attacks.stab.origin = {-10.f, -26.f};

	// if (variant == ImpVariant::knife) { visual.sprite.setTexture(svc.assets.get_texture("enemy_knife_imp")); }
	parts.weapon.animated_sprite->set_params("idle");

	cooldowns.awaken.start();
}

void Imp::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	if (directions.actual.lnr == LNR::left) {
		attacks.stab.set_position(Enemy::get_collider().physics.position);
		attacks.stab.origin.x = -10.f;
	} else {
		sf::Vector2f dir_offset{Enemy::get_collider().bounding_box.get_dimensions().x, 0.f};
		attacks.stab.set_position(Enemy::get_collider().physics.position + dir_offset);
		attacks.stab.origin.x = 10.f;
	}

	cooldowns.post_jump.update();
	cooldowns.post_attack.update();

	attacks.stab.update();
	attacks.stab.handle_player(player);

	flags.state.set(StateFlags::vulnerable); // imp is always vulnerable

	Enemy::update(svc, map, player);
	if (!is_dormant()) {
		parts.weapon.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
		parts.hand.update(svc, map, player, directions.actual, Drawable::get_scale(), get_collider().get_center());
	}
	parts.weapon.set_hitbox();

	if (svc.ticker.every_x_ticks(200)) {
		if (random::percent_chance(4) && !caution.danger()) { state = ImpState::run; }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.frdog.set(audio::Frdog::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (is_hostile() && !cooldowns.post_attack.running()) { state = ImpState::attack; }
	// if (is_hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) { state = ImpState::jump; } // player is already in hostile range
	if (!get_collider().grounded()) { state = ImpState::fall; }

	if (is_alert() && !is_hostile() && svc.ticker.every_x_ticks(32)) {
		if (random::percent_chance(50)) {
			state = ImpState::run;
		} else {
			state = ImpState::jump;
		}
	}

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lnr != directions.desired.lnr) { state = ImpState::turn; }

	state_function = state_function();
}

void Imp::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (died() || state == ImpState::dormant) { return; }
	parts.weapon.render(svc, win, cam);
	parts.hand.render(svc, win, cam);
	if (svc.greyblock_mode()) {
		if (state == ImpState::attack) { attacks.stab.render(win, cam); }
	}
}

fsm::StateFunction Imp::update_idle() {
	animation.label = "idle";
	parts.weapon.animated_sprite->set_params("idle");
	attacks.stab.disable();
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
	if (change_state(ImpState::attack, attack)) { return IMP_BIND(update_attack); }
	if (change_state(ImpState::fall, fall)) { return IMP_BIND(update_fall); }
	if (change_state(ImpState::run, run)) { return IMP_BIND(update_run); }
	if (change_state(ImpState::jump, jump)) { return IMP_BIND(update_jump); }
	state = ImpState::idle;
	return IMP_BIND(update_idle);
};

fsm::StateFunction Imp::update_turn() {
	animation.label = "turn";
	// animation.log_info();
	attacks.stab.disable();
	if (animation.totally_complete()) {
		NANI_LOG_INFO(m_logger, "finished!");
		flip();
		directions.actual = directions.desired;
		state = ImpState::idle;
		animation.set_params(idle, false);
		return IMP_BIND(update_idle);
	}
	state = ImpState::turn;
	return IMP_BIND(update_turn);
};

fsm::StateFunction Imp::update_run() {
	animation.label = "run";
	if (animation.just_started()) { parts.weapon.animated_sprite->set_params("lift"); }
	if (parts.weapon.animated_sprite->complete()) { parts.weapon.animated_sprite->set_params("run"); }
	attacks.stab.disable();
	auto facing = directions.actual.lnr == LNR::left ? -1.f : 1.f;
	get_collider().physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
	if (change_state(ImpState::fall, fall)) { return IMP_BIND(update_fall); }
	state = ImpState::run;
	return IMP_BIND(update_run);
}

fsm::StateFunction Imp::update_jump() {
	animation.label = "jump";
	auto jumpsquat_frame{1};
	attacks.stab.disable();
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = random::percent_chance(50) ? -1.f : 1.f;
		if (cooldowns.post_attack.running()) { rand_jump = directions.actual.lnr == LNR::left ? 1.f : -1.f; } // always jump backwards after a attack otherwise it feels unfair
	}
	if (cooldowns.jump.running() && animation.get_frame_count() > jumpsquat_frame) { get_collider().physics.apply_force({0, -2.5f}); }
	if (!get_collider().grounded() && animation.get_frame_count() > jumpsquat_frame) { get_collider().physics.apply_force({rand_jump * 2.f, 0.f}); }
	if (animation.get_frame_count() > jumpsquat_frame) { cooldowns.jump.update(); }
	if (cooldowns.jump.is_complete()) {
		cooldowns.post_jump.start();
		if (change_state(ImpState::fall, fall)) { return IMP_BIND(update_fall); }
		if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	state = ImpState::jump;
	return IMP_BIND(update_jump);
}

fsm::StateFunction Imp::update_fall() {
	animation.label = "fall";
	attacks.stab.disable();
	if (get_collider().grounded()) {
		if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	state = ImpState::fall;
	return IMP_BIND(update_fall);
}

fsm::StateFunction Imp::update_attack() {
	animation.label = "attack";
	attacks.stab.enable();
	if (animation.just_started()) { parts.weapon.animated_sprite->set_params("attack"); }
	if (parts.weapon.animated_sprite->complete()) { parts.weapon.animated_sprite->set_params("swoosh"); }
	auto force{3.f};
	force *= directions.actual.lnr == LNR::left ? -1.f : 1.f;
	get_collider().physics.apply_force({force, 0.f});
	if (animation.complete()) {
		cooldowns.post_attack.start();
		if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	state = ImpState::attack;
	return IMP_BIND(update_attack);
}

fsm::StateFunction Imp::update_dormant() {
	animation.label = "dormant";
	parts.weapon.animated_sprite->set_params("dormant");
	is_hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable_move);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		m_map->effects.push_back(entity::Effect(*m_services, "small_explosion", get_collider().physics.position, {}, 2));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		state = ImpState::jump;
		animation.set_params(jump);
		return IMP_BIND(update_jump);
	}
	state = ImpState::dormant;
	return IMP_BIND(update_dormant);
}

bool Imp::change_state(ImpState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
