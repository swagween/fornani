#include "fornani/entities/enemy/catalog/Eyebit.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"

namespace fornani::enemy {

Eyebit::Eyebit(automa::ServiceProvider& svc, bool spawned) : Enemy(svc, "eyebit", spawned) {
	animation.set_params(idle);
	seeker_cooldown.start(2);
	collider.flags.general.set(shape::General::ignore_resolution);
	flags.general.set(GeneralFlags::transcendent);
}

void Eyebit::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	if (!seeker_cooldown.is_complete()) { seeker.set_position(collider.physics.position); }
	seeker_cooldown.update();
	flags.state.set(StateFlags::vulnerable); // eyebit is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(EyebitState::turn); }
	direction.lnr = (player.collider.physics.position.x < collider.physics.position.x) ? LNR::left : LNR::right;

	state_function = state_function();

	if (player.collider.bounding_box.overlaps(physical.hostile_range)) { seeker.set_force(0.002f); }
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
		visual.sprite.setScale({-1.f, 1.f});
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