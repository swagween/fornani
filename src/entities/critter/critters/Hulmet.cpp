
#include "Hulmet.hpp"

namespace critter {

void Hulmet::unique_update() {

	// set ranges
	if (!colliders.empty()) {
		alert_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - alert_range.dimensions.x / 2, colliders.at(0).physics.position.y - alert_range.dimensions.y / 2));
		hostile_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - hostile_range.dimensions.x / 2, colliders.at(0).physics.position.y - hostile_range.dimensions.y / 2));
	}

	// player is in hostile range
	if (svc::playerLocator.get().collider.bounding_box.SAT(hostile_range) && !svc::playerLocator.get().collider.bounding_box.SAT(alert_range) && !flags.test(Flags::charging) && !flags.test(Flags::shooting) && stats.cooldown == 0) {

		// decide randomly whether to chase the player or start shooting
		if (svc::randomLocator.get().percent_chance(0.2f)) {
			flags.set(Flags::charging);
			flags.reset(Flags::seeking);
		} else {
			current_target = svc::playerLocator.get().collider.physics.position;
			flags.reset(Flags::charging);
			seek_current_target();
			flags.set(Flags::seeking);
		}

		flags.reset(Flags::hiding);
	}

	// player is very close
	if (svc::playerLocator.get().collider.bounding_box.SAT(alert_range)) {
		flags.set(Flags::hiding);
	} else {
		flags.reset(Flags::hiding);
	}

	// player is out of range
	if (!svc::playerLocator.get().collider.bounding_box.SAT(hostile_range) && !svc::playerLocator.get().collider.bounding_box.SAT(alert_range)) { random_walk({100, 100}); }

	barrel_point = facing_lr == behavior::DIR_LR::RIGHT ? sprite_position + sf::Vector2<float>{6.f, 28.f} : sprite_position + sf::Vector2<float>(sprite_dimensions) - sf::Vector2<float>{6.f, 28.f};

	cooldown();

	state_function = state_function();
}

void Hulmet::load_data() {
	if (colliders.empty()) {
		for (int j = 0; j < num_colliders; ++j) {
			auto const& xdim = svc::dataLocator.get().hulmet["colliders"][0]["boxes"][j]["dimensions"]["x"].as<float>();
			auto const& ydim = svc::dataLocator.get().hulmet["colliders"][0]["boxes"][j]["dimensions"]["y"].as<float>();
			auto const& xpos = svc::dataLocator.get().hulmet["colliders"][0]["boxes"][j]["position"]["x"].as<float>();
			auto const& ypos = svc::dataLocator.get().hulmet["colliders"][0]["boxes"][j]["position"]["y"].as<float>();

			colliders.push_back(shape::Collider({xdim, ydim}));
			colliders.back().sprite_offset = {xpos, ypos};
			colliders.back().physics = components::PhysicsComponent({0.8f, 0.998f}, 1.f);
			colliders.back().physics.gravity = 0.1f;
			colliders.back().physics.maximum_velocity = {0.25f, 0.25f};
		}
	}

	if (hurtbox_atlas.empty()) {
		for (int i = 0; i < num_anim_frames; ++i) {
			for (int j = 0; j < num_hurtboxes; ++j) {
				auto const& xdim = svc::dataLocator.get().hulmet["hurtboxes"][i]["boxes"][j]["dimensions"]["x"].as<float>();
				auto const& ydim = svc::dataLocator.get().hulmet["hurtboxes"][i]["boxes"][j]["dimensions"]["y"].as<float>();
				auto const& xpos = svc::dataLocator.get().hulmet["hurtboxes"][i]["boxes"][j]["position"]["x"].as<float>();
				auto const& ypos = svc::dataLocator.get().hulmet["hurtboxes"][i]["boxes"][j]["position"]["y"].as<float>();

				auto const& id = svc::dataLocator.get().hulmet["hurtboxes"][i]["tile_id"].as<int>();

				hurtbox_atlas.push_back(shape::Shape());
				hurtbox_atlas.back().dimensions = {xdim, ydim};
				hurtbox_atlas.back().sprite_offset = {xpos, ypos};
				hurtbox_atlas.back().tile_id = id;

				if (i == 0) { hurtboxes.push_back(hurtbox_atlas.back()); }
			}
		}
	}

	sprite_dimensions.x = 64;
	sprite_dimensions.y = 38;
}

fsm::StateFunction Hulmet::update_idle() {
	flags.set(Flags::vulnerable);
	if (behavior.params.complete) { behavior = behavior::Behavior(behavior::hulmet_idle); }
	if (flags.test(Flags::turning)) { return BIND(update_turn); }
	if (flags.test(Flags::charging) && stats.cooldown == 0) {
		behavior.params.started = true;
		return BIND(update_charge);
	}
	if (flags.test(Flags::hiding)) {
		behavior.params.started = true;
		return BIND(update_hide);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	if (!colliders.empty()) {
		if (abs(colliders.at(0).physics.velocity.x) > 0.2f) {
			behavior.params.started = true;
			return BIND(update_run);
		}
	}
	return std::move(state_function);
}

fsm::StateFunction Hulmet::update_turn() {
	flags.set(Flags::vulnerable);
	if (behavior.params.started) {
		behavior = behavior::Behavior(behavior::hulmet_turn);
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

fsm::StateFunction Hulmet::update_charge() {
	flags.reset(Flags::seeking);
	flags.reset(Flags::running);
	flags.set(Flags::vulnerable);
	for (auto& collider : colliders) {
		collider.physics.velocity.x = 0.f;
		collider.physics.acceleration.x = 0.f;
	}
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::hulmet_charging);
		behavior.params.started = false;
		++anim_loop_count;
	}

	if (anim_loop_count > 4) {
		behavior.params.started = true;
		anim_loop_count = 0;
		flags.reset(Flags::charging);
		flags.reset(Flags::hurt);
		return svc::playerLocator.get().collider.bounding_box.SAT(hostile_range) ? BIND(update_shoot) : (flags.test(Flags::just_hurt) ? BIND(update_hurt) : BIND(update_idle));
	}
	if (flags.test(Flags::turning)) { return BIND(update_turn); }
	// charging cannot be interrupted by hurt
	if (flags.test(Flags::just_hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	flags.reset(Flags::just_hurt);
	return std::move(state_function);
}

fsm::StateFunction Hulmet::update_shoot() {
	flags.reset(Flags::seeking);
	flags.reset(Flags::running);
	flags.set(Flags::vulnerable);
	for (auto& collider : colliders) {
		collider.physics.velocity.x = 0.f;
		collider.physics.acceleration.x = 0.f;
	}
	if (behavior.params.started) {
		behavior = behavior::Behavior(behavior::hulmet_shooting);
		flags.set(Flags::weapon_fired);
		behavior.params.started = false;
	}
	if (behavior.params.complete) {
		flags.reset(Flags::hurt);
		flags.reset(Flags::shooting);
		flags.reset(Flags::weapon_fired);
		stats.cooldown = 500;
		return BIND(update_idle);
	}

	if (flags.test(Flags::turning)) { return BIND(update_turn); }
	// shooting cannot be interrupted by hurt
	if (flags.test(Flags::just_hurt)) { svc::assetLocator.get().enem_hit.play(); }
	flags.reset(Flags::just_hurt);
	return std::move(state_function);
}

fsm::StateFunction Hulmet::update_run() {
	flags.set(Flags::vulnerable);
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::hulmet_run);
		behavior.params.started = false;
	}
	if (flags.test(Flags::turning)) {
		behavior.params.started = true;
		return BIND(update_turn);
	}
	if (!colliders.empty()) {
		if (abs(colliders.at(0).physics.velocity.x) < 0.24f) {
			behavior.params.just_started = true;
			return BIND(update_idle);
		}
	}
	if (flags.test(Flags::charging)) {
		behavior.params.started = true;
		return BIND(update_charge);
	}
	if (flags.test(Flags::hiding)) {
		behavior.params.started = true;
		return BIND(update_hide);
	}
	if (flags.test(Flags::hurt)) {
		behavior.params.started = true;
		return BIND(update_hurt);
	}
	return std::move(state_function);
}

fsm::StateFunction Hulmet::update_hurt() {
	flags.reset(Flags::seeking);
	flags.reset(Flags::running);
	flags.set(Flags::vulnerable);
	if (flags.test(Flags::turning)) { return BIND(update_turn); }
	for (auto& collider : colliders) {
		collider.physics.velocity.x = 0.f;
		collider.physics.acceleration.x = 0.f;
	}
	if (flags.test(Flags::just_hurt)) { svc::assetLocator.get().enem_hit.play(); }
	flags.reset(Flags::just_hurt);
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::hulmet_hurt);
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

fsm::StateFunction Hulmet::update_hide() {
	flags.reset(Flags::seeking);
	flags.reset(Flags::running);
	flags.reset(Flags::vulnerable);
	for (auto& collider : colliders) {
		collider.physics.velocity.x = 0.f;
		collider.physics.acceleration.x = 0.f;
	}
	if (behavior.start()) {
		behavior = behavior::Behavior(behavior::hulmet_hiding);
		behavior.params.started = false;
		++anim_loop_count;
	}
	if (anim_loop_count > 1) {
		behavior.params.current_frame = 1;
		return flags.test(Flags::hiding) ? BIND(update_hide) : BIND(update_idle);
	}
	if (flags.test(Flags::charging)) {
		behavior.params.started = true;
		flags.reset(Flags::hiding);
		return BIND(update_charge);
	}
	return std::move(state_function);
}

} // namespace critter
