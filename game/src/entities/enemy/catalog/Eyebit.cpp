
#include <fornani/entities/enemy/catalog/Eyebit.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Eyebit::Eyebit(automa::ServiceProvider& svc, world::Map& map, bool spawned) : Enemy(svc, map, "eyebit", spawned) {
	m_params = {{"idle", {0, 4, 28, -1}}, {"turn", {4, 1, 38, 0}}};
	animation.set_params(get_params("idle"));
	seeker_cooldown.start(2);
	flags.general.set(GeneralFlags::transcendent);
	flags.state.set(StateFlags::vulnerable);
}

void Eyebit::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	if (!seeker_cooldown.is_complete()) { seeker.set_position(get_collider().physics.position); }
	seeker_cooldown.update();

	face_player(player);

	if (player.get_collider().bounding_box.overlaps(physical.hostile_range)) { seeker.set_force(0.002f); }
	if (player.get_collider().bounding_box.overlaps(physical.alert_range)) {
		seeker.update(svc);
		seeker.seek_player(player);
		get_collider().physics.position = seeker.get_position();
		get_collider().physics.velocity = seeker.get_velocity();
		get_collider().sync_components();
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(EyebitState::turn); }
	Enemy::update(svc, map, player);
	seeker.set_position(get_collider().physics.position);
	state_function = state_function();
}

fsm::StateFunction Eyebit::update_idle() {
	p_state.actual = EyebitState::idle;
	if (change_state(EyebitState::turn, get_params("turn"))) { return EYEBIT_BIND(update_turn); }
	return EYEBIT_BIND(update_idle);
}

fsm::StateFunction Eyebit::update_turn() {
	p_state.actual = EyebitState::turn;
	if (animation.complete()) {
		request_flip();
		request(EyebitState::idle);
		if (change_state(EyebitState::idle, get_params("idle"))) { return EYEBIT_BIND(update_idle); }
	}
	return EYEBIT_BIND(update_turn);
}

bool Eyebit::change_state(EyebitState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
