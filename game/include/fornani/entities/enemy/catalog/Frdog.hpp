#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#define FRDOG_BIND(f) std::bind(&Frdog::f, this)

namespace fornani::enemy {

	enum class AnimState { idle, turn, run, hurt, charge, bark, spew, sit, sleep };

class Frdog : public Enemy {

  public:
	Frdog(automa::ServiceProvider& svc);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Frdog::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_sleep();
	fsm::StateFunction update_sit();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_charge();
	fsm::StateFunction update_run();
	fsm::StateFunction update_hurt();
	fsm::StateFunction update_bark();
	fsm::StateFunction update_spew();

	private:
	util::BitFlags<AnimState> state{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 2, 28, -1};
	anim::Parameters turn{13, 1, 38, 0};
	anim::Parameters hurt{9, 2, 20, 2};
	anim::Parameters charge{5, 2, 20, 4};
	anim::Parameters sleep{11, 2, 30, -1};
	anim::Parameters sit{2, 2, 30, -1};
	anim::Parameters spew{7, 2, 20, 0};
	anim::Parameters run{16, 4, 24, -1};
	anim::Parameters bark{15, 1, 22, 0};

};

} // namespace enemy
