#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Seeker.hpp"
#define EYEBOT_BIND(f) std::bind(&Eyebot::f, this)

namespace enemy {

	enum class EyebotState { idle, turn };

class Eyebot : public Enemy {

  public:
	Eyebot() = default;
	Eyebot(automa::ServiceProvider& svc);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Eyebot::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

	private:
	util::BitFlags<EyebotState> state{};

	  // packages
	  entity::Seeker seeker{0.0005f, 0.99f};
	  util::Cooldown seeker_cooldown{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{4, 1, 38, 0};

};

} // namespace enemy