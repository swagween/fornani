
#include "fornani/entities/enemy/catalog/Eyebot.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Eyebot::Eyebot(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "eyebot") {
	animation.set_params(idle);
	seeker_cooldown.start(2);
}

void Eyebot::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	if (just_died()) {
		for (int i{0}; i < 3; ++i) {
			auto const randx = random::random_range_float(-60.f, 60.f);
			auto const randy = random::random_range_float(-60.f, 60.f);
			sf::Vector2 const rand_vec{randx, randy};
			sf::Vector2f const spawn = get_collider().physics.position + rand_vec;
			map.spawn_enemy(5, spawn);
		}
	}

	if (died()) {
		update(svc, map, player);
		return;
	}
	seeker_cooldown.update();
	flags.state.set(StateFlags::vulnerable); // eyebot is always vulnerable

	// reset animation states to determine next animation state
	directions.desired.lnr = (player.get_collider().physics.position.x < get_collider().physics.position.x) ? LNR::left : LNR::right;

	state_function = state_function();

	if (player.get_collider().bounding_box.overlaps(physical.alert_range)) { get_collider().sync_components(); }

	Enemy::update(svc, map, player);
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
		flip();
		state = {};
		state.set(EyebotState::idle);
		animation.set_params(idle);
		return EYEBOT_BIND(update_idle);
	}
	state = {};
	state.set(EyebotState::turn);
	return EYEBOT_BIND(update_turn);
};

} // namespace fornani::enemy
