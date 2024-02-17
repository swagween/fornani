
#include "Frdog.hpp"
#include "../../../setup/ServiceLocator.hpp"

namespace critter {

void Frdog::unique_update() {

	if (!colliders.empty()) {
		alert_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - alert_range.dimensions.x / 2, colliders.at(0).physics.position.y - alert_range.dimensions.y / 2));
		hostile_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - hostile_range.dimensions.x / 2, colliders.at(0).physics.position.y - hostile_range.dimensions.y / 2));
	}

	if (svc::playerLocator.get().collider.bounding_box.SAT(hostile_range)) {
		current_target = svc::playerLocator.get().collider.physics.position;
		awake();
	} else if (svc::playerLocator.get().collider.bounding_box.SAT(alert_range)) {
		wake_up();
	} else {
		sleep();
	}

	// seek_current_target();
	random_idle_action();
	while (!idle_action_queue.empty()) {
		flags.set(Flags::barking);
		idle_action_queue.pop();
	}

	state_function = state_function();
}

void Frdog::load_data() {

	if (colliders.empty()) {
		for (int j = 0; j < num_colliders; ++j) {
			auto const& xdim = svc::dataLocator.get().frdog["colliders"][0]["boxes"][j]["dimensions"]["x"].as<float>();
			auto const& ydim = svc::dataLocator.get().frdog["colliders"][0]["boxes"][j]["dimensions"]["y"].as<float>();
			auto const& xpos = svc::dataLocator.get().frdog["colliders"][0]["boxes"][j]["position"]["x"].as<float>();
			auto const& ypos = svc::dataLocator.get().frdog["colliders"][0]["boxes"][j]["position"]["y"].as<float>();

			colliders.push_back(shape::Collider({xdim, ydim}));
			colliders.back().sprite_offset = {xpos, ypos};
			colliders.back().physics = components::PhysicsComponent({0.8f, 0.997f}, 1.f);
		}
	}

	if (hurtbox_atlas.empty()) {
		for (int i = 0; i < num_anim_frames; ++i) {
			for (int j = 0; j < num_hurtboxes; ++j) {
				auto const& xdim = svc::dataLocator.get().frdog["hurtboxes"][i]["boxes"][j]["dimensions"]["x"].as<float>();
				auto const& ydim = svc::dataLocator.get().frdog["hurtboxes"][i]["boxes"][j]["dimensions"]["y"].as<float>();
				auto const& xpos = svc::dataLocator.get().frdog["hurtboxes"][i]["boxes"][j]["position"]["x"].as<float>();
				auto const& ypos = svc::dataLocator.get().frdog["hurtboxes"][i]["boxes"][j]["position"]["y"].as<float>();

				auto const& id = svc::dataLocator.get().frdog["hurtboxes"][i]["tile_id"].as<int>();

				hurtbox_atlas.push_back(shape::Shape());
				hurtbox_atlas.back().dimensions = {xdim, ydim};
				hurtbox_atlas.back().sprite_offset = {xpos, ypos};
				hurtbox_atlas.back().tile_id = id;

				if (i == 0) { hurtboxes.push_back(hurtbox_atlas.back()); }
			}
		}
	}

	sprite_dimensions.x = 72;
	sprite_dimensions.y = 48;
}

fsm::StateFunction Frdog::update_idle() {
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::frdog_idle);
		behavior.params.started = false;
	}
	if (!colliders.empty()) {
		if (abs(colliders.at(0).physics.velocity.x) > 0.0002f) {
			behavior.params.started = true;
			return BIND(update_run);
		}
	}
	if (flags.test(Flags::awakened)) {
		behavior.params.started = true;
		return BIND(update_sit);
	}
	if (flags.test(Flags::turning)) {
		behavior.params.started = true;
		return BIND(update_turn);
	}
	if (flags.test(Flags::barking)) {
		behavior.params.started = true;
		return BIND(update_bark);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_sleep() {

	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::frdog_asleep);
		behavior.params.started = false;
	}
	if (flags.test(Flags::awakened)) {
		behavior.params.started = true;
		return BIND(update_sit);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_sit() {
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::frdog_awakened);
		behavior.params.started = false;
	}
	if (!flags.test(Flags::awakened)) {
		behavior.params.started = true;
		return flags.test(Flags::asleep) ? BIND(update_sleep) : BIND(update_idle);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_turn() {
	if (behavior.params.started) {
		behavior = behavior::Behavior(behavior::frdog_turn);
		behavior.params.started = false;
	}
	if (behavior.params.complete) {
		flags.reset(Flags::turning);
		flags.set(Flags::flip);
		return BIND(update_idle);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_charge() {
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_run() {
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::frdog_run);
		behavior.params.started = false;
	}
	if (flags.test(Flags::turning)) {
		behavior.params.started = true;
		return BIND(update_turn);
	}
	if (!colliders.empty()) {
		if (abs(colliders.at(0).physics.velocity.x) < 0.04f) {
			behavior.params.just_started = true;
			return BIND(update_idle);
		}
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Frdog::update_hurt() {
	if (flags.test(Flags::just_hurt)) { svc::assetLocator.get().enem_hit.play(); }
	flags.reset(Flags::just_hurt);
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::frdog_hurt);
		behavior.params.started = false;
		++anim_loop_count;
	}

	if (anim_loop_count > 2) {
		behavior.params.started = true;
		anim_loop_count = 0;
		flags.reset(Flags::hurt);
		return flags.test(Flags::shot) ? BIND(update_hurt) : BIND(update_idle);
	}

	return std::move(state_function);
}

fsm::StateFunction Frdog::update_bark() {
	if (behavior.params.started) {
		behavior = behavior::Behavior(behavior::frdog_bark);
		behavior.params.started = false;
	}
	if (behavior.params.complete) {
		behavior.params.started = true;
		flags.reset(Flags::barking);
		return BIND(update_idle);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

} // namespace critter