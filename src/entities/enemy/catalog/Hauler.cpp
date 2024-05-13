#include "Hauler.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Hauler::Hauler(automa::ServiceProvider& svc) : Enemy(svc, "hauler") , m_services(&svc) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
}

void Hauler::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	gun.update(svc, map, *this);
	caution.avoid_ledges(map, collider, 1);
	running_time.update();

	if (state.test(HaulerState::haul)) {
		//haul grenade
	}

	// reset animation states to determine next animation state
	state = {};
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Enemy::update(svc, map, player);

	if (hostility_triggered() && gun.clip_cooldown.is_complete() && running_time.is_complete()) {
		state.set(HaulerState::alert);
	}
	if (hostile() && !hostility_triggered() && gun.clip_cooldown.is_complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger(direction)) {
			state.set(HaulerState::haul);
		} else {
			state.set(HaulerState::run);
			running_time.start(400);
		}
	} // player is already in hostile range

	if (caution.danger(direction)) { running_time.cancel(); }
	if (running_time.is_complete() && gun.clip_cooldown.is_complete()) {
		state.set(HaulerState::idle);
	} else if (!running_time.is_complete()) {
		state = {};
		state.set(HaulerState::run);
	}

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(4) && !caution.danger(direction)) {
			state.set(HaulerState::run);
			running_time.start(400);
		}
	}

	if(flags.state.test(StateFlags::hurt)) {
		if (m_services->random.percent_chance(50)) {
			m_services->soundboard.flags.tank.set(audio::Hauler::hurt_1);
		} else {
			m_services->soundboard.flags.tank.set(audio::Hauler::hurt_2);
		}
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hurt_effect.running()) {
		if ((hurt_effect.get_cooldown() / 32) % 2 == 0) {
			sprite.setColor(flcolor::red);
		} else {
			sprite.setColor(flcolor::periwinkle);
		}
	} else {
		sprite.setColor(flcolor::white);
	}

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Hauler::death); }

	if (ent_state.test(entity::State::flip)) { state.set(HaulerState::turn); }

	state_function = state_function();
}

fsm::StateFunction Hauler::update_idle() {
	animation.label = "idle";
	if (state.test(HaulerState::alert)) {
		state.reset(HaulerState::idle);
		animation.set_params(alert);
		return TANK_BIND(update_alert);
	}
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::idle);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	if (state.test(HaulerState::haul)) {
		state.reset(HaulerState::idle);
		animation.set_params(haul);
		return TANK_BIND(update_haul);
	}
	if (state.test(HaulerState::run)) {
		state.reset(HaulerState::idle);
		animation.set_params(run);
		return TANK_BIND(update_run);
	}
	state = {};
	state.set(HaulerState::idle);
	return TANK_BIND(update_idle);
};
fsm::StateFunction Hauler::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		sprite_flip();
		state = {};
		state.set(HaulerState::idle);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	state = {};
	state.set(HaulerState::turn);
	return TANK_BIND(update_turn);
};
fsm::StateFunction Hauler::update_run() {
	animation.label = "run";
	auto facing = direction.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger(direction)) { running_time.cancel(); }
	if (running_time.is_complete()) { state.set(HaulerState::idle); }
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::run);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	if (state.test(HaulerState::idle)) {
		state.reset(HaulerState::run);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (state.test(HaulerState::alert)) {
		state.reset(HaulerState::run);
		animation.set_params(alert);
		return TANK_BIND(update_alert);
	}
	state = {};
	state.set(HaulerState::run);
	return TANK_BIND(update_run);
}

fsm::StateFunction Hauler::update_haul() {
	animation.label = "haul";
	if (animation.complete() && animation.keyframe_over()) {
		gun.clip_cooldown.start(gun.clip_cooldown_time);
		state = {};
		state.set(HaulerState::idle);
		state.reset(HaulerState::haul);
		animation.set_params(idle);
		return TANK_BIND(update_idle);
	}
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::haul);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
	}
	state = {};
	state.set(HaulerState::haul);
	return TANK_BIND(update_haul);
}

fsm::StateFunction Hauler::update_alert() { 
	animation.label = "alert";
	if (animation.just_started()) {
		m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
	}
	if (animation.complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger(direction)) {
			state.set(HaulerState::haul);
		} else {
			state.set(HaulerState::run);
			running_time.start(400);
		}
		if (state.test(HaulerState::haul)) {
			state.reset(HaulerState::idle);
			animation.set_params(haul);
			return TANK_BIND(update_haul);
		}
		if (state.test(HaulerState::run)) {
			state.reset(HaulerState::idle);
			animation.set_params(run);
			return TANK_BIND(update_run);
		}
	}
	state = {};
	state.set(HaulerState::alert);
	return TANK_BIND(update_alert);
}
fsm::StateFunction Hauler::update_jump() { return fsm::StateFunction(); }
fsm::StateFunction Hauler::update_hurt() { return fsm::StateFunction(); };

} // namespace enemy