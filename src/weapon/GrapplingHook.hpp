
#pragma once

#include "../particle/Spring.hpp"
#include "../utils/Cooldown.hpp"

namespace arms {
enum class GrappleTriggers { found, released };
enum class GrappleState { anchored, probing, snaking };
class GrapplingHook {
  public:
	void update();
	vfx::Spring spring{};
	util::BitFlags<GrappleTriggers> grapple_triggers{};
	util::BitFlags<GrappleState> grapple_flags{};
};
} // namespace arms
