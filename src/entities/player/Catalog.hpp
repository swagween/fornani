
#pragma once
#include "../../utils/BitFlags.hpp"
#include "AbilityManager.hpp"
#include "Inventory.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace player {

class Catalog {
  public:
	void update(automa::ServiceProvider& svc);
	void add_item(automa::ServiceProvider& svc, int item_id, int amount);
	void remove_item(automa::ServiceProvider& svc, int item_id, int amount);
	struct {
		Inventory inventory{};
		AbilityManager abilities{};
	} categories{};
};

} // namespace player
