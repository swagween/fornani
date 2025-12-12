
#pragma once

#include "fornani/components/CircleSensor.hpp"
#include "fornani/entities/Entity.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/physics/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Direction.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::arms {
enum class GrenadeFlags { detonated };
// TODO: re-implement grenade to fit into new weapons system
} // namespace fornani::arms
