
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {
class Dash : public Ability {
  public:
	Dash(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction, bool omni);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_vertical_multiplier;
	float m_horizontal_multiplier;
	int m_rate;
	bool m_omni;
};

} // namespace fornani::player
