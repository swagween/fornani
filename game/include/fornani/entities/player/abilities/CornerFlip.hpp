
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {
class CornerFlip : public Ability {
  public:
	CornerFlip(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_vertical_multiplier;
	util::Cooldown m_beginning;
};
} // namespace fornani::player
