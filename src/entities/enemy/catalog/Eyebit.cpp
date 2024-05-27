#include "Eyebit.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Eyebit::Eyebit(automa::ServiceProvider& svc) : Enemy(svc, "eyebit") {
	animation.set_params(idle);
	seeker_cooldown.start(2);
}

void Eyebit::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (!seeker_cooldown.is_complete()) { seeker.set_position(collider.physics.position); }
	seeker_cooldown.update();
	flags.state.set(StateFlags::vulnerable); // eyebit is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(EyebitState::turn); }
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;

	state_function = state_function();

	if (collider.has_horizontal_collision()) { seeker.bounce_horiz(); }
	if (collider.has_vertical_collision()) { seeker.bounce_vert(); }

	if (player.collider.bounding_box.overlaps(physical.hostile_range)) { seeker.set_force(0.004f); }
	if (player.collider.bounding_box.overlaps(physical.alert_range)) {
		seeker.update(svc);
		seeker.seek_player(player);
		collider.physics.position = seeker.get_position();
		collider.physics.velocity = seeker.get_velocity();
		collider.sync_components();
	}

	Enemy::update(svc, map, player);
	seeker.set_position(collider.physics.position);
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