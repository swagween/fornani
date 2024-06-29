#include "Tank.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Tank::Tank(automa::ServiceProvider& svc) : Enemy(svc, "tank"), gun(svc, "skycorps ar", 2) , m_services(&svc) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
}

void Tank::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	gun.update(svc, map, *this);
	caution.avoid_ledges(map, collider, 1);
	running_time.update();

	if (state.test(TankState::shoot)) {
		if (!gun.get().cooling_down()) {
			gun.cycle.update();
			gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{-14.f, 10.f} : sf::Vector2<float>{26.f, 20.f};
			gun.shoot();
			map.spawn_projectile_at(svc, gun.get(), gun.barrel_point());
			m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
		}
	}

	// reset animation states to determine next animation state
	state = {};
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Enemy::update(svc, map, player);

	if (hostility_triggered() && gun.clip_cooldown.is_complete() && running_time.is_complete()) {
		state.set(TankState::alert);
	}
	if (hostile() && !hostility_triggered() && gun.clip_cooldown.is_complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger(direction)) {
			state.set(TankState::shoot);
		} else {
			state.set(TankState::run);
			running_time.start(400);
		}
	} // player is already in hostile range

	if (caution.danger(direction)) { running_time.cancel(); }
	if (running_time.is_complete() && gun.clip_cooldown.is_complete()) {
		state.set(TankState::idle);
	} else if (!running_time.is_complete()) {
		state = {};
		state.set(TankState::run);
	}

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(4) && !caution.danger(direction)) {
			state.set(TankState::run);
			running_time.start(400);
		}
	}

	if(flags.state.test(StateFlags::hurt)) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::hurt_2);
		}
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hurt_effect.running()) {
		if ((hurt_effect.get_cooldown() / 32) % 2 == 0) {
			sprite.setColor(svc.styles.colors.red);
		} else {
			sprite.setColor(svc.styles.colors.periwinkle);
		}
	} else {
		sprite.setColor(svc.styles.colors.white);
	}

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Tank::death); }

	if (ent_state.test(entity::State::flip)) { state.set(TankState::turn); }

	state_function = state_function();
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	if (state.test(TankState::alert)) {
		state.reset(TankState::idle);
		animation.set_params(alert);
		return TANK_BIND(update_alert);
	}
	if (state.test(TankState::turn)) {
		state.reset(TankState::idle);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	if (state.test(TankState::shoot)) {
		state.reset(TankState::idle);
		animation.set_params(shoot);
		return TANK_BIND(update_shoot);
	}
	if (state.test(TankState::run)) {
		state.reset(TankState::idle);
		animation.set_params(run);
		return TANK_BIND(update_run);
	}
	state = {};
	state.set(TankState::idle);
	return TANK_BIND(update_idle);
};
fsm::StateFunction Tank::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		sprite_flip();
		state = {};
		state.set(TankState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	state = {};
	state.set(TankState::turn);
	return TANK_BIND(update_turn);
};
fsm::StateFunction Tank::update_run() {
	animation.label = "run";
	auto facing = direction.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger(direction)) { running_time.cancel(); }
	if (running_time.is_complete()) { state.set(TankState::idle); }
	if (state.test(TankState::turn)) {
		state.reset(TankState::run);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	if (state.test(TankState::idle)) {
		state.reset(TankState::run);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (state.test(TankState::alert)) {
		state.reset(TankState::run);
		animation.set_params(alert);
		return TANK_BIND(update_alert);
	}
	state = {};
	state.set(TankState::run);
	return TANK_BIND(update_run);
}
fsm::StateFunction Tank::update_shoot() {
	animation.label = "shoot";
	if (animation.complete() && animation.keyframe_over()) {
		gun.clip_cooldown.start(gun.clip_cooldown_time);
		state = {};
		state.set(TankState::idle);
		state.reset(TankState::shoot);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (state.test(TankState::turn)) {
		state.reset(TankState::shoot);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	state = {};
	state.set(TankState::shoot);
	return TANK_BIND(update_shoot);
}

fsm::StateFunction Tank::update_alert() { 
	animation.label = "alert";
	if (animation.just_started()) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Tank::alert_2);
		}
	}
	if (animation.complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger(direction)) {
			state.set(TankState::shoot);
		} else {
			state.set(TankState::run);
			running_time.start(400);
		}
		if (state.test(TankState::shoot)) {
			state.reset(TankState::idle);
			animation.set_params(shoot);
			return TANK_BIND(update_shoot);
		}
		if (state.test(TankState::run)) {
			state.reset(TankState::idle);
			animation.set_params(run);
			return TANK_BIND(update_run);
		}
	}
	state = {};
	state.set(TankState::alert);
	return TANK_BIND(update_alert);
};

} // namespace enemy