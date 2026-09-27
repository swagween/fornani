
#pragma once

#include <fornani/entities/enemy/Enemy.hpp>
#define EYEBOT_BIND(f) std::bind(&Eyebot::f, this)

namespace fornani::enemy {

enum class EyebotState : std::uint8_t { idle, turn };

class Eyebot final : public Enemy, StateMachine<EyebotState> {

  public:
	explicit Eyebot(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;

	fsm::StateFunction state_function = std::bind(&Eyebot::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();

  private:
	bool change_state(EyebotState next, anim::Parameters params);

  private:
	components::SteeringBehavior m_steering{};
};

} // namespace fornani::enemy
