#pragma once

#include "../Enemy.hpp"
#define TANK_BIND(f) std::bind(&Tank::f, this)

namespace enemy {

	enum class TankState { idle, turn, run, shoot };

class Tank : public Enemy {

  public:
	Tank() = default;
	Tank(automa::ServiceProvider& svc);
	void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Tank::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_run();
	fsm::StateFunction update_shoot();

	private:
	util::BitFlags<TankState> state{};

	//packages
	entity::WeaponPackage gun;
	util::Cooldown running_time{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 28, -1};
	anim::Parameters turn{6, 3, 38, 0};
	anim::Parameters run{9, 4, 38, -1};
	anim::Parameters shoot{13, 3, 22, 6};

};

} // namespace enemy