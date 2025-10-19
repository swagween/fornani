
#pragma once
#include "fornani/entities/item/Item.hpp"

#include <memory>
#include <vector>

namespace fornani::player {

class Inventory {
  public:
	/// @brief Adds item to inventory.
	/// @returns the item's integer ID.
	int add_item(dj::Json& source, std::string_view label);
	void remove_item(int item_id, int amount);
	void reveal_item(int item_id);
	[[nodiscard]] bool has_item(int id) const;
	[[nodiscard]] bool has_item(std::string_view label) const;
	std::vector<std::unique_ptr<item::Item>> const& items_view() const { return m_items; }
	item::Item& item_view(int id) const&;

  private:
	std::vector<std::unique_ptr<item::Item>> m_items{};
};

} // namespace fornani::player
