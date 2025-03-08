
#pragma once
#include "fornani/entities/item/types/ApparelItem.hpp"
#include "fornani/entities/item/types/KeyItem.hpp"

#include <memory>
#include <vector>

namespace fornani::player {

class Inventory {
  public:
	int add_item(dj::Json& source, std::string_view label, item::ItemType type); // returns item id
	void remove_item(int item_id, int amount);
	void reveal_item(int item_id);
	[[nodiscard]] bool has_item(int id) const;
	std::vector<std::unique_ptr<item::ApparelItem>> const& apparel_view() const { return m_apparel; };
	std::vector<std::unique_ptr<item::KeyItem>> const& key_items_view() const { return m_key_items; };
	item::Item& item_view(int id) const&;

  private:
	std::vector<std::unique_ptr<item::ApparelItem>> m_apparel{};
	std::vector<std::unique_ptr<item::KeyItem>> m_key_items{};
};

} // namespace fornani::player
