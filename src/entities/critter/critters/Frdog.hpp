//
//  frdog.hpp
//  critter
//
//

#pragma once

#include "../Critter.hpp"
#define BIND(f) std::bind(&Frdog:: f, this)

namespace critter {

	class Frdog : public Critter {
	
	public:

		Frdog() {
			num_anim_frames = 20;
			num_colliders = 1;
			num_hurtboxes = 2;
			offset.y = 2.f;
			metadata = { 0, VARIANT::BEAST, true, false, true };
			stats = { 3, 35, 0.3f, 2.0f, 80, 12 * 32 };
			spritesheet_dimensions = sf::Vector2<int>{4, 5};
			init();
			flags.set(Flags::vulnerable); //frdogs always vulnerable

			alert_range = shape::Shape({ (float)stats.vision * 1.5f, (float)stats.vision * 1.5f });
			hostile_range = shape::Shape({ (float)stats.vision, (float)stats.vision });
		}

		void unique_update() override;
		void load_data() override;

		fsm::StateFunction state_function = std::bind(&Frdog::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_sleep();
		fsm::StateFunction update_sit();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_charge();
		fsm::StateFunction update_run();
		fsm::StateFunction update_hurt();
		fsm::StateFunction update_bark();



	};

} // end critter

/* frdog_hpp */
