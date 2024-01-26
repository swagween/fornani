//
//  hulmet.hpp
//  critter
//
//

#pragma once

#include "../Critter.hpp"
#define BIND(f) std::bind(&Hulmet:: f, this)

namespace critter {

	class Hulmet : public Critter {

	public:

		Hulmet() {
			metadata = { 1, VARIANT::SOLDIER, true, false, true }; stats = { 2, 14, 0.5f, 1.0f, 16 * 32 };
			sprite_dimensions = { 56, 42 };
			spritesheet_dimensions = { 1, 14 };
			dimensions = { 16.0f, 16.0f }; 
			colliders.push_back(shape::Collider());
			init();
			flags.vulnerable = true; //hulmets always vulnerable
		}

		void unique_update() override;
		void load_data() override;
		
		fsm::StateFunction state_function = std::bind(&Hulmet::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_alert();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_charge();
		fsm::StateFunction update_run();
		fsm::StateFunction update_hurt();
		fsm::StateFunction update_die();

	};

} // end critter

/* hulmet_hpp */
