#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {
class Roll : public Ability {
  public:
	Roll(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_multiplier;
	util::Cooldown m_request;
	audio::Soundboard* m_soundboard;
};
} // namespace fornani::player
