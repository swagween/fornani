#pragma once

#include "fornani/entities/enemy/Enemy.hpp"
#define HAULER_BIND(f) std::bind(&Hauler::f, this)

namespace enemy {

	enum class HaulerState { idle, turn, run, haul, alert, jump, hurt };

class Hauler : public Enemy {

	  public:
		Hauler(automa::ServiceProvider& svc);
		void unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

		fsm::StateFunction state_function = std::bind(&Hauler::update_idle, this);
		fsm::StateFunction update_idle();
		fsm::StateFunction update_turn();
		fsm::StateFunction update_run();
		fsm::StateFunction update_haul();
		fsm::StateFunction update_alert();
		fsm::StateFunction update_jump();
		fsm::StateFunction update_hurt();

	  private:
		util::BitFlags<HaulerState> state{};

	float fire_chance{80.f};

	//packages
	entity::Caution caution{};
	util::Cooldown running_time{};
	util::Cooldown hurt_effect{};

	// lookup, duration, framerate, num_loops
	anim::Parameters idle{0, 6, 28, -1};
	anim::Parameters turn{6, 3, 38, 0};
	anim::Parameters run{9, 4, 38, -1};
	anim::Parameters haul{13, 3, 22, 3};
	anim::Parameters alert{17, 3, 42, 0};
	anim::Parameters jump{17, 3, 42, 0};
	anim::Parameters hurt{17, 3, 42, 0};

	automa::ServiceProvider* m_services;

};

} // namespace enemy
