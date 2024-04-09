#include "Eyebit.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Eyebit::Eyebit(automa::ServiceProvider& svc) : Enemy(svc, "eyebit") {}

void Eyebit::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	
	flags.state.set(StateFlags::vulnerable); // frdog is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(EyebitState::turn); }
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::right : dir::LR::left;

	state_function = state_function();
	Enemy::update(svc, map);
}

fsm::StateFunction Eyebit::update_idle() {
	animation.label = "idle";
	if (state.test(EyebitState::turn)) {
		state.reset(EyebitState::idle);
		animation.set_params(turn);
		return EYEBIT_BIND(update_turn);
	}
	state = {};
	state.set(EyebitState::idle);
	return std::move(state_function);
};
fsm::StateFunction Eyebit::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		sprite_flip();
		state = {};
		state.set(EyebitState::idle);
		animation.set_params(idle);
		return EYEBIT_BIND(update_idle);
	}
	state = {};
	state.set(EyebitState::turn);
	return EYEBIT_BIND(update_turn);
};
} // namespace enemy