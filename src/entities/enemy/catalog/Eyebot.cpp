#include "Eyebot.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Eyebot::Eyebot(automa::ServiceProvider& svc) : Enemy(svc, "eyebot") { animation.set_params(idle); }

void Eyebot::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	
	flags.state.set(StateFlags::vulnerable); // frdog is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(EyebotState::turn); }
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::right : dir::LR::left;

	state_function = state_function();
	Enemy::update(svc, map);
}

fsm::StateFunction Eyebot::update_idle() {
	animation.label = "idle";
	if (state.test(EyebotState::turn)) {
		state.reset(EyebotState::idle);
		animation.set_params(turn);
		return EYEBOT_BIND(update_turn);
	}
	state = {};
	state.set(EyebotState::idle);
	return std::move(state_function);
};
fsm::StateFunction Eyebot::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		sprite_flip();
		state = {};
		state.set(EyebotState::idle);
		animation.set_params(idle);
		return EYEBOT_BIND(update_idle);
	}
	state = {};
	state.set(EyebotState::turn);
	return EYEBOT_BIND(update_turn);
};
} // namespace enemy