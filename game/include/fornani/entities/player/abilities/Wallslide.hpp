
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {

class Wallslide : public Ability {
  public:
	Wallslide(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_speed_multiplier;
	float m_base_grav;
	world::Map* m_map;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::player
