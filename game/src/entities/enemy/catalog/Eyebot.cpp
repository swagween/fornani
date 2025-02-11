#include "fornani/entities/enemy/catalog/Eyebot.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::enemy {

Eyebot::Eyebot(automa::ServiceProvider& svc) : Enemy(svc, "eyebot") {
	animation.set_params(idle);
	seeker_cooldown.start(2);
}

void Eyebot::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	if (just_died()) {
		for (int i{0}; i < 3; ++i) {
			auto const randx = util::Random::random_range_float(-60.f, 60.f);
			auto const randy = util::Random::random_range_float(-60.f, 60.f);
			sf::Vector2 const rand_vec{randx, randy};
			sf::Vector2<float> const spawn = collider.physics.position + rand_vec;
			map.spawn_enemy(5, spawn);
		}
	}

	if (died()) {
		update(svc, map, player);
		return;
	}
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

	Enemy::update(svc, map, player);
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
		visual.sprite.scale({-1.f, 1.f});
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