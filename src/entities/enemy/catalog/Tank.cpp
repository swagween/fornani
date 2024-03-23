#include "Tank.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Tank::Tank(automa::ServiceProvider& svc) : Enemy(svc, "tank") { animation.set_params(idle); }

void Tank::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	
	flags.state.set(StateFlags::vulnerable); // frdog is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(TankState::turn); }
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::right : dir::LR::left;

	state_function = state_function();
	Enemy::update(svc, map);
}

fsm::StateFunction Tank::update_idle() {
	animation.label = "idle";
	if (state.test(TankState::turn)) {
		state.reset(TankState::idle);
		animation.set_params(turn);
		return TANK_BIND(update_turn);
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
fsm::StateFunction Tank::update_shoot() { return TANK_BIND(update_idle); };

} // namespace enemy