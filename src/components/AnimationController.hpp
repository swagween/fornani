//
//  AnimationController.hpp
//  components
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include "../utils/StateMachine.hpp"
#include "../components/Animation.hpp"

namespace components {

	const int animation_multiplier{ 32 };

class AnimationController {

public:

	void refresh();
	void update();
	int get_frame();

	fsm::StateMachine<Animation> sm{};

	Time dt{ 0.001f };
	Clock::time_point current_time = Clock::now();
	Time accumulator{ 0.0f };
};

} // end components

/* AnimationController_hpp */
