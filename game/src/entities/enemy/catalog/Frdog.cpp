#include "fornani/entities/enemy/catalog/Frdog.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"

namespace fornani::enemy {

Frdog::Frdog(automa::ServiceProvider& svc) : Enemy(svc, "frdog") { animation.set_params(idle); }

void Frdog::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	
	flags.state.set(StateFlags::vulnerable); // frdog is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	if (ent_state.test(entity::State::flip)) { state.set(AnimState::turn); }
	direction.lnr = (player.collider.physics.position.x < collider.physics.position.x) ? LNR::right : LNR::left;
	if (flags.state.test(StateFlags::hurt)) {
		state.set(AnimState::hurt);
		flags.state.reset(StateFlags::hurt);
	}

	state_function = state_function();
	Enemy::update(svc, map, player);
}

fsm::StateFunction Frdog::update_idle() {
	animation.label = "idle";
	if (state.test(AnimState::turn)) {
		state.reset(AnimState::idle);
		animation.set_params(turn);
		return FRDOG_BIND(update_turn);
	}
	if (state.test(AnimState::hurt)) {
		state.reset(AnimState::idle);
		animation.set_params(hurt);
		return FRDOG_BIND(update_hurt);
	}
	state = {};
	state.set(AnimState::idle);
	return std::move(state_function);
};
fsm::StateFunction Frdog::update_sleep() { return FRDOG_BIND(update_idle); };
fsm::StateFunction Frdog::update_sit() { return FRDOG_BIND(update_idle); };
fsm::StateFunction Frdog::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.scale({-1.f, 1.f});
		if (state.test(AnimState::hurt)) {
			state.reset(AnimState::turn);
			animation.set_params(hurt);
			return FRDOG_BIND(update_hurt);
		}
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return FRDOG_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::turn);
	return FRDOG_BIND(update_turn);
};
fsm::StateFunction Frdog::update_charge() { return FRDOG_BIND(update_idle); };
fsm::StateFunction Frdog::update_run() { return FRDOG_BIND(update_idle); };
fsm::StateFunction Frdog::update_hurt() {
	animation.label = "hurt";
	if (animation.complete()) {
		if (state.test(AnimState::hurt)) {
			state.reset(AnimState::hurt);
			animation.set_params(hurt);
			return FRDOG_BIND(update_hurt);
		}
		if (state.test(AnimState::turn)) {
			state.reset(AnimState::turn);
			animation.set_params(turn);
			return FRDOG_BIND(update_turn);
		}
		state = {};
		state.set(AnimState::idle);
		animation.set_params(idle);
		return FRDOG_BIND(update_idle);
	}
	state = {};
	state.set(AnimState::hurt);
	return std::move(state_function);
};
fsm::StateFunction Frdog::update_bark() { return FRDOG_BIND(update_idle); };
fsm::StateFunction Frdog::update_spew() { return FRDOG_BIND(update_idle); };

} // namespace enemy