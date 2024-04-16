#include "Eyebot.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Eyebot::Eyebot(automa::ServiceProvider& svc) : Enemy(svc, "eyebot") {
	animation.set_params(idle);
	seeker_cooldown.start(2);
}

void Eyebot::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (!seeker_cooldown.is_complete()) { seeker.set_position(collider.physics.position); }
	seeker_cooldown.update();
	flags.state.set(StateFlags::vulnerable); // eyebot is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(EyebotState::turn); }
	direction.lr = (player.collider.physics.position.x < collider.physics.position.x) ? dir::LR::left : dir::LR::right;

	state_function = state_function();

	if (collider.has_horizontal_collision()) { seeker.bounce_horiz(); }
	if (collider.has_vertical_collision()) { seeker.bounce_vert(); }

	if (player.collider.bounding_box.overlaps(physical.hostile_range)) { seeker.set_force(0.002f); }
	if (player.collider.bounding_box.overlaps(physical.alert_range)) {
		seeker.update(svc);
		seeker.seek_player(player);
		collider.physics.position = seeker.get_position();
		collider.physics.velocity = seeker.get_velocity();
		collider.sync_components();
	}

	Enemy::update(svc, map);
	seeker.set_position(collider.physics.position);
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