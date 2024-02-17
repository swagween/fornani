
#pragma once

#include "../Critter.hpp"
#define BIND(f) std::bind(&Hulmet::f, this)

namespace critter {

class Hulmet : public Critter {

  public:
	Hulmet() {
		num_anim_frames = 20;
		num_colliders = 1;
		num_hurtboxes = 1;
		offset.y = 8.f;
		metadata = {1, VARIANT::SOLDIER, true, false, true};
		stats = {2, 14, 0.5f, 1.0f, 16 * 32, 16 * 32};
		spritesheet_dimensions = sf::Vector2<int>{5, 4};
		init();
		flags.set(Flags::vulnerable); // hulmets always vulnerable

		alert_range = shape::Shape({(float)stats.vision / 4, (float)stats.vision * 2});
		hostile_range = shape::Shape({(float)stats.vision, (float)stats.vision / 6});
		stats.cooldown = 500;

		weapon = arms::skycorps_ar;
		weapon.projectile.team = arms::TEAMS::SKYCORPS;
	}

	void unique_update() override;
	void load_data() override;

	fsm::StateFunction state_function = std::bind(&Hulmet::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_shoot();
	fsm::StateFunction update_run();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_hide();
};

} // namespace critter
