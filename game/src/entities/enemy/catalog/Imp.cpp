#include "fornani/entities/enemy/catalog/Imp.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::enemy {

Imp::Imp(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "imp"), m_services(&svc), m_map(&map),
	  parts{.knife{svc.assets.t_imp_knife, {72, 40}, {{0, 1, 32, -1}, {1, 1, 32, 0}, {2, 1, 32, -1}, {3, 3, 32, 0}, {5, 1, 32, 0}, {6, 1, 32, 0}}, {"idle", "lift", "run", "attack", "dormant", "swoosh"}, 2.0f, 0.85f, {0.f, 0.f}},
			.fork{svc.assets.t_imp_fork, {82, 34}, {{0, 1, 32, -1}, {1, 1, 32, 0}, {2, 1, 32, -1}, {3, 3, 32, 9}, {0, 1, 32, -1}, {6, 1, 32, 0}}, {"idle", "lift", "run", "attack", "dormant", "swoosh"}, 2.0f, 0.85f, {0.f, 0.f}},
			.hand{svc.styles.colors.ui_black, {4.f, 4.f}, 2.0f, 0.85f}} {
	animation.set_params(dormant);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	secondary_collider = shape::Collider({22.f, 22.f});
	directions.desired.lr = dir::LR::left;
	directions.actual.lr = dir::LR::left;
	directions.movement.lr = dir::LR::neutral;
	attacks.stab.sensor.bounds.setRadius(10);
	attacks.stab.sensor.drawable.setFillColor(svc.styles.colors.blue);
	attacks.stab.hit.bounds.setRadius(28);
	attacks.stab.origin = {-10.f, -26.f};

	variant = util::Random::percent_chance(50) ? ImpVariant::fork : ImpVariant::knife;
	if (variant == ImpVariant::knife) { visual.sprite.setTexture(svc.assets.t_knife_imp); }
	parts.fork.animated_sprite->set_params("idle");
	parts.knife.animated_sprite->set_params("idle");

	cooldowns.awaken.start();
}

void Imp::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	if (directions.actual.lr == dir::LR::left) {
		attacks.stab.set_position(Enemy::collider.physics.position);
		attacks.stab.origin.x = -10.f;
	} else {
		sf::Vector2<float> dir_offset{Enemy::collider.bounding_box.get_dimensions().x, 0.f};
		attacks.stab.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.stab.origin.x = 10.f;
	}

	attacks.stab.disable();

	cooldowns.post_jump.update();
	cooldowns.post_attack.update();

	attacks.stab.update();
	attacks.stab.handle_player(player);

	flags.state.set(StateFlags::vulnerable); // imp is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 3);

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	if (!is_dormant()) {
		parts.knife.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
		parts.fork.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
		parts.hand.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
	}
	if (variant == ImpVariant::fork) { parts.fork.set_hitbox(); }
	if (variant == ImpVariant::knife) { parts.knife.set_hitbox(); }

	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{0.f, 10.f};
	secondary_collider.physics.position.x += directions.actual.lr == dir::LR::left ? 2.f : collider.dimensions.x - secondary_collider.dimensions.x - 2.f;
	secondary_collider.sync_components();
	if (player.collider.hurtbox.overlaps(secondary_collider.bounding_box) && !is_dormant()) { player.hurt(); }

	if (svc.ticker.every_x_ticks(200)) {
		if (util::Random::percent_chance(4) && !caution.danger()) { state = ImpState::run; }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.frdog.set(audio::Frdog::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hostile() && !cooldowns.post_attack.running()) { state = ImpState::fall; }
	if (hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) { state = ImpState::jump; } // player is already in hostile range

	if (alert() && !hostile() && svc.ticker.every_x_ticks(32)) {
		if (util::Random::percent_chance(50)) {
			state = ImpState::run;
		} else {
			state = ImpState::jump;
		}
	}

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lr != directions.desired.lr) { state = ImpState::turn; }

	state_function = state_function();
}

void Imp::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (died() || state == ImpState::dormant) { return; }
	switch (variant) {
	case ImpVariant::fork: parts.fork.render(svc, win, cam); break;
	case ImpVariant::knife: parts.knife.render(svc, win, cam); break;
	}
	parts.hand.render(svc, win, cam);
	if (svc.greyblock_mode()) {
		if (state == ImpState::attack) { attacks.stab.render(win, cam); }
	}
}

fsm::StateFunction Imp::update_idle() {
	animation.label = "idle";
	parts.fork.animated_sprite->set_params("idle");
	parts.knife.animated_sprite->set_params("idle");
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(ImpState::turn, turn)) { return IMP_BIND(update_turn); }
	if (change_state(ImpState::fall, fall)) { return IMP_BIND(update_fall); }
	if (change_state(ImpState::run, run)) { return IMP_BIND(update_run); }
	if (change_state(ImpState::jump, jump)) { return IMP_BIND(update_jump); }
	state = ImpState::idle;
	return IMP_BIND(update_idle);
};

fsm::StateFunction Imp::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.scale({-1.f, 1.f});
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
	auto facing = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
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
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = util::Random::percent_chance(50) ? -1.f : 1.f;
		if (cooldowns.post_attack.running()) { rand_jump = directions.actual.lr == dir::LR::left ? 1.f : -1.f; } // always jump backwards after a attack otherwise it feels unfair
	}
	if (cooldowns.jump.running()) { collider.physics.apply_force({0, -2.5f}); }
	if (!collider.grounded()) { collider.physics.apply_force({rand_jump * 2.f, 0.f}); }
	cooldowns.jump.update();
	if (cooldowns.jump.is_complete() && collider.grounded()) {
		cooldowns.post_jump.start();
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	state = ImpState::jump;
	return IMP_BIND(update_jump);
}

fsm::StateFunction Imp::update_fall() {
	animation.label = "fall";
	shake();
	if (animation.complete()) {
		if (directions.actual.lr != directions.desired.lr) {
			state = ImpState::turn;
			animation.set_params(turn);
			return IMP_BIND(update_turn);
		}
		state = ImpState::attack;
		animation.set_params(attack);
		return IMP_BIND(update_attack);
	}
	state = ImpState::fall;
	return IMP_BIND(update_fall);
}

fsm::StateFunction Imp::update_attack() {
	attacks.stab.enable();
	if (caution.danger()) {
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	}
	auto force{16.f};
	force *= directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({force, 0.f});
	if (animation.complete()) {
		cooldowns.post_attack.start();
		state = ImpState::idle;
		animation.set_params(idle);
		return IMP_BIND(update_idle);
	};
	state = ImpState::attack;
	return IMP_BIND(update_attack);
}

fsm::StateFunction Imp::update_dormant() {
	hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		m_map->effects.push_back(entity::Effect(*m_services, collider.physics.position, {}, 2, 0));
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
