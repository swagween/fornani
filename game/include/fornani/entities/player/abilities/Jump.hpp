
#pragma once

#include <fornani/entities/player/abilities/Ability.hpp>

namespace fornani::player {

class Jump : public Ability {
  public:
	Jump(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider);
	void update(shape::Collider& collider, PlayerController& controller) override;

  private:
	float m_multiplier;
	util::Cooldown m_request;
	util::Cooldown m_post_jump;
	audio::Soundboard* m_soundboard;
	world::Map* m_map;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::player
