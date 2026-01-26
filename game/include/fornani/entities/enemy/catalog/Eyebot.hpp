#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Seeker.hpp"
#define EYEBOT_BIND(f) std::bind(&Eyebot::f, this)

namespace fornani::enemy {

enum class EyebotState { idle, turn };

class Eyebot final : public Enemy {

  public:
	explicit Eyebot(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Eyebot::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

  private:
	util::BitFlags<EyebotState> state{};

	// packages
	util::Cooldown seeker_cooldown{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 4, 28, -1};
	anim::Parameters turn{4, 1, 38, 0};
};

} // namespace fornani::enemy
