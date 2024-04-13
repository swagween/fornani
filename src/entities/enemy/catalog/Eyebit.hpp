#pragma once

#include "../Enemy.hpp"
#include "../../packages/Seeker.hpp"
#define EYEBIT_BIND(f) std::bind(&Eyebit::f, this)

namespace enemy {

	enum class EyebitState { idle, turn };

class Eyebit : public Enemy {

  public:
	Eyebit() = default;
	Eyebit(automa::ServiceProvider& svc);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Eyebit::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

	private:
	util::BitFlags<EyebitState> state{};

	//packages
	entity::Seeker seeker{0.001f, 0.992f};
	util::Cooldown seeker_cooldown{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{4, 1, 38, 0};

};

} // namespace enemy