
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#include <fornani/entities/packages/Seeker.hpp>
#define EYEBIT_BIND(f) std::bind(&Eyebit::f, this)

namespace fornani::enemy {

enum class EyebitState { idle, turn };

class Eyebit final : public Enemy, public StateMachine<EyebitState> {

  public:
	explicit Eyebit(automa::ServiceProvider& svc, world::Map& map, bool spawned = false);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Eyebit::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

  private:
	bool change_state(EyebitState next, anim::Parameters params);

	// packages
	entity::Seeker seeker{0.001f, 0.99f};
	util::Cooldown seeker_cooldown{};
};

} // namespace fornani::enemy
