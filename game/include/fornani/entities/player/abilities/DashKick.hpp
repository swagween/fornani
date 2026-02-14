
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {
class DashKick : public Ability {
  public:
	DashKick(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_vertical_multiplier;
	float m_horizontal_multiplier;
	float original_gravity{};
	int m_rate;
};

} // namespace fornani::player
