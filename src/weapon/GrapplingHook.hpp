
#pragma once

#include "../particle/Spring.hpp"
#include "../utils/Cooldown.hpp"
#include "../utils/Direction.hpp"

namespace arms {
enum class GrappleTriggers { found, released };
enum class GrappleState { anchored, probing, snaking };
class GrapplingHook {
  public:
	void update();
	sf::Vector2<float> probe_velocity(float speed);
	vfx::Spring spring{};
	util::BitFlags<GrappleTriggers> grapple_triggers{};
	util::BitFlags<GrappleState> grapple_flags{};
	dir::Direction probe_direction{};
};
} // namespace arms
