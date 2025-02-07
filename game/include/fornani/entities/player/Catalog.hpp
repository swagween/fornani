
#pragma once
#include "fornani/utils/BitFlags.hpp"
#include "AbilityManager.hpp"
#include "Inventory.hpp"
#include "Wardrobe.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

class Catalog {
  public:
	void update(automa::ServiceProvider& svc);
	void add_item(automa::ServiceProvider& svc, int item_id, int amount);
	void remove_item(automa::ServiceProvider& svc, int item_id, int amount);
	void equip_item(automa::ServiceProvider& svc, ApparelType type, int item_id);
	void unequip_item(ApparelType type);
	struct {
		Inventory inventory{};
		AbilityManager abilities{};
		Wardrobe wardrobe{};
	} categories{};
};

} // namespace fornani::player
