
#pragma once
#include "fornani/entities/player/AbilityManager.hpp"
#include "fornani/entities/player/Inventory.hpp"
#include "fornani/entities/player/Wardrobe.hpp"

namespace fornani::player {

struct Catalog {
	Inventory inventory;
	AbilityManager abilities{};
	Wardrobe wardrobe{};
};

} // namespace fornani::player
