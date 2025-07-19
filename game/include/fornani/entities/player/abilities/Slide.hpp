
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {

class Slide : public Ability {
  public:
	Slide(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_speed_multiplier;
	float m_dampen;
	float m_minimum_threshold;
	world::Map* m_map;
	util::Cooldown m_post_slide{};
};

} // namespace fornani::player
