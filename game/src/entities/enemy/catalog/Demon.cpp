#include "fornani/entities/enemy/catalog/Demon.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::enemy {

Demon::Demon(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "demon"), m_services(&svc), m_map(&map),
	  parts{.spear{svc.assets.t_demon_spear, 2.0f, 0.85f, {-16.f, 8.f}}, .sword{svc.assets.t_demon_sword, 2.0f, 0.85f, {-4.f, 8.f}}, .shield{svc.assets.t_demon_shield, 2.0f, 0.85f, {-28.f, 8.f}}} {
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

	attacks.rush.sensor.bounds.setRadius(40);
	attacks.rush.hit.bounds.setRadius(40);
	attacks.rush.origin = {20.f, 16.f};
	attacks.rush.hit_offset = {0.f, 0.f};

	variant = util::Random::percent_chance(50) ? DemonVariant::spearman : DemonVariant::warrior;
	if (variant == DemonVariant::spearman) { health.set_max(56); }

	cooldowns.awaken.start();
}

void Demon::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	if (directions.actual.lr == dir::LR::left) {
		attacks.stab.set_position(Enemy::collider.physics.position);
		attacks.rush.set_position(Enemy::collider.physics.position);
		attacks.stab.origin.x = -10.f;
		attacks.rush.origin.x = 20.f;
		attacks.rush.hit_offset.x = 0.f;
	} else {
		sf::Vector2<float> dir_offset{Enemy::collider.bounding_box.get_dimensions().x, 0.f};
		attacks.stab.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.rush.set_position(Enemy::collider.physics.position + dir_offset);
		attacks.stab.origin.x = 10.f;
		attacks.rush.origin.x = -20.f;
		attacks.rush.hit_offset.x = 0.f;
	}

	cooldowns.rush_hit.update();
	cooldowns.post_jump.update();
	cooldowns.post_rush.update();

	attacks.stab.update();
	attacks.rush.update();
	attacks.stab.handle_player(player);
	attacks.rush.handle_player(player);
	if (state == DemonState::rush && attacks.rush.sensor.active() && !cooldowns.rush_hit.running()) {
		auto sign = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
		if ((sign == -1.f && player_behind(player)) || (sign == 1.f && !player_behind(player))) {
			// player.hurt(1);
			player.accumulated_forces.push_back({sign * 2.f, -2.f});
			attacks.rush.sensor.deactivate();
			cooldowns.rush_hit.start();
		}
	}

	flags.state.set(StateFlags::vulnerable); // demon is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 3);

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	if (!is_dormant()) {
		parts.spear.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
		parts.sword.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
		parts.shield.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center());
	}
	if (variant == DemonVariant::spearman) { parts.spear.set_hitbox(); }
	if (variant == DemonVariant::warrior) {
		parts.sword.set_hitbox();
		parts.shield.set_shield();
	}

	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{0.f, 10.f};
	secondary_collider.physics.position.x += directions.actual.lr == dir::LR::left ? 2.f : collider.dimensions.x - secondary_collider.dimensions.x - 2.f;
	secondary_collider.sync_components();
	if (player.collider.hurtbox.overlaps(secondary_collider.bounding_box) && !is_dormant()) { player.hurt(); }

	if (svc.ticker.every_x_ticks(200)) {
		if (util::Random::percent_chance(4) && !caution.danger()) { state = DemonState::run; }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.demon.set(audio::Demon::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hostile() && !cooldowns.post_rush.running()) { state = DemonState::signal; }
	if (hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) { state = DemonState::jumpsquat; } // player is already in hostile range

	if (alert() && !hostile() && svc.ticker.every_x_ticks(32)) {
		if (util::Random::percent_chance(50)) {
			state = DemonState::run;
		} else {
			state = DemonState::jumpsquat;
		}
	}

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lr != directions.desired.lr) { state = DemonState::turn; }

	state_function = state_function();
}

void Demon::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (died() || state == DemonState::dormant) { return; }
	if (variant == DemonVariant::spearman) {
		parts.spear.render(svc, win, cam);
	} else {
		parts.shield.render(svc, win, cam);
		parts.sword.render(svc, win, cam);
	}
	if (svc.greyblock_mode()) {
		if (state == DemonState::stab) { attacks.stab.render(win, cam); }
		attacks.rush.render(win, cam);
	}
}

fsm::StateFunction Demon::update_idle() {
	animation.label = "idle";
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(DemonState::turn, turn)) { return DEMON_BIND(update_turn); }
	if (change_state(DemonState::signal, signal)) { return DEMON_BIND(update_signal); }
	if (change_state(DemonState::run, run)) { return DEMON_BIND(update_run); }
	if (change_state(DemonState::jumpsquat, jumpsquat)) { return DEMON_BIND(update_jumpsquat); }
	state = DemonState::idle;
	return DEMON_BIND(update_idle);
};

fsm::StateFunction Demon::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.scale({-1.f, 1.f});
		directions.actual = directions.desired;
		state = DemonState::idle;
		animation.set_params(idle, false);
		return DEMON_BIND(update_idle);
	}
	state = DemonState::turn;
	return DEMON_BIND(update_turn);
};

fsm::StateFunction Demon::update_run() {
	animation.label = "run";
	auto facing = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		state = DemonState::idle;
		animation.set_params(idle);
		return DEMON_BIND(update_idle);
	}
	if (change_state(DemonState::turn, turn)) { return DEMON_BIND(update_turn); }
	if (change_state(DemonState::signal, signal)) { return DEMON_BIND(update_signal); }
	state = DemonState::run;
	return DEMON_BIND(update_run);
}

fsm::StateFunction Demon::update_jump() {
	animation.label = "jump";
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = util::Random::percent_chance(50) ? -1.f : 1.f;
		if (cooldowns.post_rush.running()) { rand_jump = directions.actual.lr == dir::LR::left ? 1.f : -1.f; } // always jump backwards after a rush otherwise it feels unfair
	}
	if (cooldowns.jump.running()) { collider.physics.apply_force({0, -2.5f}); }
	if (!collider.grounded()) { collider.physics.apply_force({rand_jump * 2.f, 0.f}); }
	cooldowns.jump.update();
	if (cooldowns.jump.is_complete() && collider.grounded()) {
		cooldowns.post_jump.start();
		state = DemonState::idle;
		animation.set_params(idle);
		return DEMON_BIND(update_idle);
	}
	state = DemonState::jump;
	return DEMON_BIND(update_jump);
}

fsm::StateFunction Demon::update_signal() {
	animation.label = "signal";
	if (animation.just_started()) { m_services->soundboard.flags.demon.set(audio::Demon::signal); }
	shake();
	if (animation.complete()) {
		if (directions.actual.lr != directions.desired.lr) {
			state = DemonState::turn;
			animation.set_params(turn);
			return DEMON_BIND(update_turn);
		}
		state = DemonState::rush;
		animation.set_params(rush);
		return DEMON_BIND(update_rush);
	}
	state = DemonState::signal;
	return DEMON_BIND(update_signal);
}

fsm::StateFunction Demon::update_rush() {
	if (caution.danger()) {
		state = DemonState::idle;
		animation.set_params(idle);
		return DEMON_BIND(update_idle);
	}
	auto force{16.f};
	force *= directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({force, 0.f});
	directions.actual.lr == dir::LR::left ? parts.sword.move({-70.f, 0.f}) : parts.sword.move({70.f, 0.f});
	directions.actual.lr == dir::LR::left ? parts.spear.move({-70.f, 0.f}) : parts.spear.move({70.f, 0.f});
	if (animation.complete()) {
		cooldowns.post_rush.start();
		state = DemonState::idle;
		animation.set_params(idle);
		return DEMON_BIND(update_idle);
	};
	state = DemonState::rush;
	return DEMON_BIND(update_rush);
}

fsm::StateFunction Demon::update_stab() { return DEMON_BIND(update_idle); }

fsm::StateFunction Demon::update_dormant() {
	flags.state.reset(StateFlags::vulnerable);
	hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		flags.state.set(StateFlags::vulnerable);
		m_map->effects.push_back(entity::Effect(*m_services, collider.physics.position, {}, 2, 0));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		m_services->soundboard.flags.demon.set(audio::Demon::snort);
		state = DemonState::jump;
		animation.set_params(jump);
		return DEMON_BIND(update_jump);
	}
	state = DemonState::dormant;
	return DEMON_BIND(update_dormant);
}

fsm::StateFunction Demon::update_jumpsquat() {
	if (animation.just_started()) { m_services->soundboard.flags.demon.set(audio::Demon::snort); }
	if (animation.complete()) {
		state = DemonState::jump;
		animation.set_params(jump);
		return DEMON_BIND(update_jump);
	}
	state = DemonState::jumpsquat;
	return DEMON_BIND(update_jumpsquat);
}

bool Demon::change_state(DemonState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy