
#pragma once
#include "../../utils/BitFlags.hpp"
#include "../item/Item.hpp"
#include <vector>
#include <unordered_map>

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace player {

class Inventory {
  public:
	Inventory();
	void update(automa::ServiceProvider& svc);
	void add_item(automa::ServiceProvider& svc, int item_id, int amount);
	void clear();
	std::vector<item::Item> items{};
	std::unordered_map<int, std::string_view> item_labels{};

};

} // namespace player
