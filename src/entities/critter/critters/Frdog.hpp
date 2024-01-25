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
			metadata = { 0, VARIANT::BEAST, true, false, true };
			stats = { 3, 35, 0.3f, 2.0f, 80, 12 * 32 };
			//sprite_dimensions = sf::Vector2<int>{ 72, 48 };
			load_data();
			spritesheet_dimensions = sf::Vector2<int>{4, 5};
			dimensions = sf::Vector2<float>{32.0f, 32.0f};
			offset.y = 8;
			init();
			flags.vulnerable = true; //frdogs always vulnerable
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
