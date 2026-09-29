
#include "fornani/entities/enemy/catalog/Eyebot.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Eyebot::Eyebot(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "eyebot") {
	p_animatable.set_animations({{"idle", {0, 4, 32, -1}}, {"turn", {4, 1, 32, 0}}});
	p_animatable.animation.set_params(get_params("idle"));
	flags.state.set(StateFlags::vulnerable); // eyebot is always vulnerable
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	get_collider().physics.set_friction_componentwise({0.98f, 0.98f});
	flags.general.reset(GeneralFlags::gravity);
}

void Eyebot::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (just_died()) {
		for (int i{0}; i < 3; ++i) {
			sf::Vector2f const spawn = get_collider().get_center() + random::random_vector_float(-6.f, 6.f);
			map.spawn_enemy(5, spawn, true);
		}
	}
	Enemy::update(svc, map, player);
	if (died()) { return; }

	face_player(player);

	auto force = is_hostile() ? 0.0002f : 0.0001f;
	m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center(), force);

	// reset animation states to determine next animation state
	if (directions.actual.lnr != directions.desired.lnr) { request(EyebotState::turn); }

	state_function = state_function();
}

fsm::StateFunction Eyebot::update_idle() {
	p_state.actual = EyebotState::idle;
	if (change_state(EyebotState::turn, get_params("turn"))) { return EYEBOT_BIND(update_turn); }
	return EYEBOT_BIND(update_idle);
};

fsm::StateFunction Eyebot::update_turn() {
	p_state.actual = EyebotState::turn;
	if (p_animatable.animation.is_complete()) {
		request_flip();
		request(EyebotState::idle);
		if (change_state(EyebotState::idle, get_params("idle"))) { return EYEBOT_BIND(update_idle); }
	}
	return EYEBOT_BIND(update_turn);
}

bool Eyebot::change_state(EyebotState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
};

} // namespace fornani::enemy
