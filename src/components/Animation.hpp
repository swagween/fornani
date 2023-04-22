//
//  PhysicsComponent.hpp
//  components
//
//

#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <random>
#include <chrono>
#include <array>

namespace components {

	const int DEFAULT_FRAMERATE{ 10 };

	using Clock = std::chrono::steady_clock;
	using Time = std::chrono::duration<float>;

	class Animation {
	public:

		Animation() = default;
		Animation(const std::string& lbl, int d, int f, bool t, bool r, bool nl, int lookup) : label(lbl), duration(d), framerate(f), transitional(t), restrictive(r), no_loop(nl), lookup_value(lookup) {}

		std::string_view label{};
		int duration{};
		int framerate{};
		bool transitional{};
		bool restrictive{};
		bool no_loop{};
		int current_frame{};
		int anim_frame{};
		int lookup_value{};
		bool complete{};
		bool done{};

		bool frame_trigger{};

	};

	/*inline Animation frdog_idle = Animation("frdog_idle", 2, 12, false, false, false, 0);
	inline Animation frdog_asleep = Animation("frdog_asleep", 2, 64, true, false, false, 11);
	inline Animation frdog_charge = Animation("frdog_charge", 2, DEFAULT_FRAMERATE, false, false, false, 5);
	inline Animation frdog_bite = Animation("frdog_bite", 2, DEFAULT_FRAMERATE, false, false, true, 7);
	inline Animation frdog_hurt = Animation("frdog_hurt", 2, DEFAULT_FRAMERATE, false, false, false, 9);
	inline Animation frdog_sit = Animation("frdog_sit", 2, 12, true, false, true, 2);
	inline Animation frdog_bark = Animation("frdog_bark", 1, 16, true, false, true, 15);
	inline Animation frdog_turn = Animation("frdog_turn", 1, 12, true, true, true, 13);
	inline Animation frdog_run = Animation("frdog_run", 4, DEFAULT_FRAMERATE, false, false, false, 16);

	class FrdogIdle : Animation { std::array<Animation&, 6> valid_transitions{ frdog_run, frdog_hurt, frdog_turn, frdog_charge, frdog_sit, frdog_bark }; };
	class FrdogRun : Animation { std::array<Animation, 2> valid_transitions{ frdog_idle, frdog_hurt }; };
	class FrdogTurn : Animation { std::array<Animation, 3> valid_transitions{ frdog_idle, frdog_hurt, frdog_run }; };
	class FrdogCharge : Animation { std::array<Animation, 1> valid_transitions{ frdog_bite }; };
	class FrdogBite : Animation { std::array<Animation, 1> valid_transitions{ frdog_idle }; };
	class FrdogSleep : Animation { std::array<Animation, 1> valid_transitions{ frdog_sit }; };
	class FrdogSit : Animation { std::array<Animation, 2> valid_transitions{ frdog_idle, frdog_asleep }; };
	class FrdogBark : Animation { std::array<Animation, 2> valid_transitions{ frdog_idle, frdog_hurt }; };*/


} // end components

/* PhysicsComponent_hpp */
