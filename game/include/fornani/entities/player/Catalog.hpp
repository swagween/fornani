
#pragma once

#include <fornani/entities/player/Inventory.hpp>
#include <fornani/entities/player/PhotoAlbum.hpp>
#include <fornani/entities/player/Wardrobe.hpp>

namespace fornani::player {

struct Catalog {
	Inventory inventory;
	PhotoAlbum album{};
	Wardrobe wardrobe{};
};

} // namespace fornani::player
