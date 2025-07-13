
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::player {
class Doublejump : public Ability {
  public:
	Doublejump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	util::Cooldown m_start{12};
	float m_vertical_multiplier;
};
} // namespace fornani::player
