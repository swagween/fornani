#include "fornani/entities/enemy/catalog/Hauler.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"

namespace enemy {

Hauler::Hauler(automa::ServiceProvider& svc) : Enemy(svc, "hauler") , m_services(&svc) {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {3.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
}

void Hauler::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 1);
	running_time.update();

	if (state.test(HaulerState::haul)) {
		//haul grenade
	}

	// reset animation states to determine next animation state
	state = {};
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Enemy::update(svc, map, player);

	if (hostility_triggered() && running_time.is_complete()) {
		state.set(HaulerState::alert);
	}
	if (hostile() && !hostility_triggered()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger()) {
			state.set(HaulerState::haul);
		} else {
			state.set(HaulerState::run);
			running_time.start(400);
		}
	} // player is already in hostile range

	if (caution.danger()) { running_time.cancel(); }
	if (running_time.is_complete()) {
		state.set(HaulerState::idle);
	} else if (!running_time.is_complete()) {
		state = {};
		state.set(HaulerState::run);
	}

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(4) && !caution.danger()) {
			state.set(HaulerState::run);
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
			visual.sprite.setColor(svc.styles.colors.red);
		} else {
			visual.sprite.setColor(svc.styles.colors.periwinkle);
		}
	} else {
		visual.sprite.setColor(svc.styles.colors.white);
	}

	if (just_died()) { m_services->soundboard.flags.tank.set(audio::Tank::death); }

	if (ent_state.test(entity::State::flip)) { state.set(HaulerState::turn); }

	state_function = state_function();
}

fsm::StateFunction Hauler::update_idle() {
	animation.label = "idle";
	if (state.test(HaulerState::alert)) {
		state.reset(HaulerState::idle);
		animation.set_params(alert);
		return HAULER_BIND(update_alert);
	}
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::idle);
		animation.set_params(turn);
		return HAULER_BIND(update_turn);
	}
	if (state.test(HaulerState::haul)) {
		state.reset(HaulerState::idle);
		animation.set_params(haul);
		return HAULER_BIND(update_haul);
	}
	if (state.test(HaulerState::run)) {
		state.reset(HaulerState::idle);
		animation.set_params(run);
		return HAULER_BIND(update_run);
	}
	state = {};
	state.set(HaulerState::idle);
	return HAULER_BIND(update_idle);
};
fsm::StateFunction Hauler::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.setScale({-1.f, 1.f});
		state = {};
		state.set(HaulerState::idle);
		animation.set_params(idle);
		return HAULER_BIND(update_idle);
	}
	state = {};
	state.set(HaulerState::turn);
	return HAULER_BIND(update_turn);
};
fsm::StateFunction Hauler::update_run() {
	animation.label = "run";
	auto facing = direction.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger()) { running_time.cancel(); }
	if (running_time.is_complete()) { state.set(HaulerState::idle); }
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::run);
		animation.set_params(turn);
		return HAULER_BIND(update_turn);
	}
	if (state.test(HaulerState::idle)) {
		state.reset(HaulerState::run);
		animation.set_params(idle);
		return HAULER_BIND(update_idle);
	}
	if (state.test(HaulerState::alert)) {
		state.reset(HaulerState::run);
		animation.set_params(alert);
		return HAULER_BIND(update_alert);
	}
	state = {};
	state.set(HaulerState::run);
	return HAULER_BIND(update_run);
}

fsm::StateFunction Hauler::update_haul() {
	animation.label = "haul";
	if (animation.complete() && animation.keyframe_over()) {
		state = {};
		state.set(HaulerState::idle);
		state.reset(HaulerState::haul);
		animation.set_params(idle);
		return HAULER_BIND(update_idle);
	}
	if (state.test(HaulerState::turn)) {
		state.reset(HaulerState::haul);
		animation.set_params(turn);
		return HAULER_BIND(update_turn);
	}
	state = {};
	state.set(HaulerState::haul);
	return HAULER_BIND(update_haul);
}

fsm::StateFunction Hauler::update_alert() { 
	animation.label = "alert";
	if (animation.just_started()) {
		m_services->soundboard.flags.tank.set(audio::Tank::alert_1);
	}
	if (animation.complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger()) {
			state.set(HaulerState::haul);
		} else {
			state.set(HaulerState::run);
			running_time.start(400);
		}
		if (state.test(HaulerState::haul)) {
			state.reset(HaulerState::idle);
			animation.set_params(haul);
			return HAULER_BIND(update_haul);
		}
		if (state.test(HaulerState::run)) {
			state.reset(HaulerState::idle);
			animation.set_params(run);
			return HAULER_BIND(update_run);
		}
	}
	state = {};
	state.set(HaulerState::alert);
	return HAULER_BIND(update_alert);
}
fsm::StateFunction Hauler::update_jump() { return HAULER_BIND(update_haul); }
fsm::StateFunction Hauler::update_hurt() { return HAULER_BIND(update_haul); };

} // namespace enemy