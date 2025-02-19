#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#include "fornani/entities/packages/Caution.hpp"
#define TANK_BIND(f) std::bind(&Tank::f, this)

namespace fornani::enemy {

enum class TankState : uint8_t { idle, turn, run, shoot, alert };

class Tank final : public Enemy {

	  public:
		Tank() = delete;
		~Tank() override {}
		Tank& operator = (Tank&&) = delete;
		Tank(automa::ServiceProvider& svc, world::Map& map);
		void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

		fsm::StateFunction state_function = std::bind(&Tank::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_run();
		fsm::StateFunction update_shoot();
		fsm::StateFunction update_alert();

	  private:
		TankState state{};

	float fire_chance{50.f};

	//packages
	entity::WeaponPackage gun;
	entity::Caution caution{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 28, -1};
	anim::Parameters turn{6, 2, 38, 0};
	anim::Parameters run{9, 4, 38, 2};
	anim::Parameters shoot{13, 3, 22, 3};
	anim::Parameters alert{17, 3, 42, 0};

	automa::ServiceProvider* m_services;
	world::Map* m_map;

	bool change_state(TankState next, anim::Parameters params);

};

} // namespace enemy