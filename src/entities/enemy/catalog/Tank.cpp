#include "Tank.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"
#include "../../../level/Map.hpp"

namespace enemy {

Tank::Tank(automa::ServiceProvider& svc) : Enemy(svc, "tank"), gun(svc, "skycorps ar", 2) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
}

void Tank::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	
	flags.state.set(StateFlags::vulnerable); // tank is always vulnerable
	gun.update(svc, map, *this);

	// reset animation states to determine next animation state
	state = {};
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Enemy::update(svc, map);
	if (ent_state.test(entity::State::flip)) { state.set(TankState::turn); }

	if (player.collider.bounding_box.overlaps(physical.hostile_range) && gun.clip_cooldown.is_complete()) { state.set(TankState::shoot); }
	if (state.test(TankState::shoot)) {
		if (!gun.get().cooling_down()) {
			gun.cycle.update();
			gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{-14.f, 10.f} : sf::Vector2<float>{26.f, 20.f};
			gun.shoot();
			map.spawn_projectile_at(svc, gun.get(), gun.barrel_point());
		}
	}

	state_function = state_function();
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
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
fsm::StateFunction Tank::update_run() { return TANK_BIND(update_idle); }
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
};

} // namespace enemy