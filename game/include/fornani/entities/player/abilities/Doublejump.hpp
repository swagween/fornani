
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {
class Doublejump : public Ability {
  public:
	Doublejump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_vertical_multiplier;
};
} // namespace fornani::player
