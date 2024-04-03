
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
	void add_item(automa::ServiceProvider& svc, int item_id, int amount);
  //private:
	struct {
		Inventory inventory{};
		AbilityManager abilities{};
	} categories{};
};

} // namespace player