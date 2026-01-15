
#include <fornani/entities/enemy/catalog/Eyebit.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::enemy {

Eyebit::Eyebit(automa::ServiceProvider& svc, world::Map& map, bool spawned) : Enemy(svc, map, "eyebit", spawned) {
	m_params = {{"idle", {0, 4, 28, -1}}, {"turn", {4, 1, 38, 0}}};
	animation.set_params(get_params("idle"));
	flags.general.set(GeneralFlags::transcendent);
	flags.state.set(StateFlags::vulnerable);
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().physics.set_friction_componentwise({0.98f, 0.98f});
}

void Eyebit::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	face_player(player);

	auto force = is_hostile() ? 0.0002f : 0.0001f;
	m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center(), force);

	if (directions.actual.lnr != directions.desired.lnr) { request(EyebitState::turn); }
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
