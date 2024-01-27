//
//  hulmet.cpp
//  critter
//
//

#include "Hulmet.hpp"

namespace critter {
	
	void Hulmet::unique_update() {

        random_walk({ 100, 100 });


        if (!colliders.empty()) {
            alert_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - alert_range.dimensions.x / 2, colliders.at(0).physics.position.y - alert_range.dimensions.y / 2));
            hostile_range.set_position(sf::Vector2<float>(colliders.at(0).physics.position.x - hostile_range.dimensions.x / 2, colliders.at(0).physics.position.y - hostile_range.dimensions.y / 2));
        }
        if(svc::playerLocator.get().collider.bounding_box.SAT(hostile_range) && !svc::playerLocator.get().collider.bounding_box.SAT(alert_range) && !flags.charging && !flags.shooting && stats.cooldown == 0) {
            flags.charging = true;
            flags.hiding = false;
            seek_current_target();
        }
        if (svc::playerLocator.get().collider.bounding_box.SAT(alert_range)) {
            flags.hiding = true;
        } else {
            flags.hiding = false;
        }

        flags.vulnerable = !flags.hiding;

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

                colliders.push_back(shape::Collider({ xdim, ydim }));
                colliders.back().sprite_offset = { xpos, ypos };
                colliders.back().physics = components::PhysicsComponent({ 0.8f, 0.998f }, 1.f);
                colliders.back().physics.gravity = 0.1f;
                colliders.back().physics.maximum_velocity = { 0.25f, 0.25f };
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
                    hurtbox_atlas.back().dimensions = { xdim, ydim };
                    hurtbox_atlas.back().sprite_offset = { xpos, ypos };
                    hurtbox_atlas.back().tile_id = id;

                    if (i == 0) {
                        hurtboxes.push_back(hurtbox_atlas.back());
                    }

                }
            }
        }

        sprite_dimensions.x = 64;
        sprite_dimensions.y = 38;
	}

	fsm::StateFunction Hulmet::update_idle() {
		if (behavior.params.complete) { behavior = behavior::Behavior(behavior::hulmet_idle); }
		if (flags.turning) { return BIND(update_turn); }
        if (flags.charging && stats.cooldown == 0) { behavior.params.started = true; return BIND(update_charge); }
        if (flags.hiding) { behavior.params.started = true; return BIND(update_hide); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        if (!colliders.empty()) {
            if (abs(colliders.at(0).physics.velocity.x) > 0.2f) { behavior.params.started = true; return BIND(update_run); }
        }
		return std::move(state_function);
	}

	fsm::StateFunction Hulmet::update_turn() {
		if (behavior.params.started) { behavior = behavior::Behavior(behavior::hulmet_turn); behavior.params.started = false; }
		if (behavior.params.complete) { flags.turning = false; flags.flip = true; return BIND(update_idle); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
		return std::move(state_function);
	}

    fsm::StateFunction Hulmet::update_charge() {
        flags.seeking = false;
        flags.running = false;
        for (auto& collider : colliders) { collider.physics.velocity.x = 0.f; collider.physics.acceleration.x = 0.f; }
        if (behavior.start()) {
            behavior = behavior::Behavior(behavior::hulmet_charging);
            behavior.params.started = false;
            ++anim_loop_count;
        }

        if (anim_loop_count > 4) {
            behavior.params.started = true;
            anim_loop_count = 0;
            flags.charging = false;
            flags.hurt = false;
            return svc::playerLocator.get().collider.bounding_box.SAT(hostile_range) ? BIND(update_shoot) : BIND(update_idle);
        }
        //charging cannot be interrupted by hurt
        if (flags.just_hurt) { svc::assetLocator.get().enem_hit.play(); }
        flags.just_hurt = false;
        return std::move(state_function);
    }

    fsm::StateFunction Hulmet::update_shoot() {
        flags.seeking = false;
        flags.running = false;
        for (auto& collider : colliders) { collider.physics.velocity.x = 0.f; collider.physics.acceleration.x = 0.f; }
        if (behavior.params.started) { behavior = behavior::Behavior(behavior::hulmet_shooting); flags.weapon_fired = true; behavior.params.started = false; }
        if (behavior.params.complete) { flags.hurt = false; flags.shooting = false; flags.weapon_fired = false; stats.cooldown = 500; return BIND(update_idle); }
        //shooting cannot be interrupted by hurt
        if (flags.just_hurt) { svc::assetLocator.get().enem_hit.play(); }
        flags.just_hurt = false;
        return std::move(state_function);
    }

    fsm::StateFunction Hulmet::update_run() {
        if (behavior.start()) { behavior = behavior::Behavior(behavior::hulmet_run); behavior.params.started = false; }
        if (flags.turning) { behavior.params.started = true; return BIND(update_turn); }
        if (!colliders.empty()) {
            if (abs(colliders.at(0).physics.velocity.x) < 0.24f) { behavior.params.just_started = true; return BIND(update_idle); }
        }
        if (flags.charging) { behavior.params.started = true; return BIND(update_charge); }
        if (flags.hiding) { behavior.params.started = true; return BIND(update_hide); }
        if (flags.hurt) { behavior.params.started = true; return BIND(update_hurt); }
        return std::move(state_function);
    }

    fsm::StateFunction Hulmet::update_hurt() {
        flags.seeking = false;
        flags.running = false;
        for (auto& collider : colliders) { collider.physics.velocity.x = 0.f; collider.physics.acceleration.x = 0.f; }
        if (flags.just_hurt) { svc::assetLocator.get().enem_hit.play(); }
        flags.just_hurt = false;
        if (behavior.start()) {
            behavior = behavior::Behavior(behavior::hulmet_hurt);
            behavior.params.started = false;
            ++anim_loop_count;
        }

        if (anim_loop_count > 2) {
            behavior.params.started = true;
            anim_loop_count = 0;
            flags.hurt = false;
            return flags.shot ? BIND(update_hurt) : BIND(update_idle);
        }

        return std::move(state_function);
    }

    fsm::StateFunction Hulmet::update_hide() {
        flags.seeking = false;
        flags.running = false;
        for (auto& collider : colliders) { collider.physics.velocity.x = 0.f; collider.physics.acceleration.x = 0.f; }
        if (behavior.start()) {
            behavior = behavior::Behavior(behavior::hulmet_hiding);
            behavior.params.started = false;
            ++anim_loop_count;
        }
        if (anim_loop_count > 1) {
            behavior.params.current_frame = 1;
            return flags.hiding ? BIND(update_hide) : BIND(update_idle);
        }
        if (flags.charging) { behavior.params.started = true; flags.hiding = false; return BIND(update_charge); }
        return std::move(state_function);
    }

} // end critter

/* hulmet_cpp */
