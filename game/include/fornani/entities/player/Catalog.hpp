
#pragma once
#include "AbilityManager.hpp"
#include "Inventory.hpp"
#include "Wardrobe.hpp"

namespace fornani::player {

struct Catalog {
	Inventory inventory;
	AbilityManager abilities{};
	Wardrobe wardrobe{};
};

} // namespace fornani::player
