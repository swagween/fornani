
#pragma once
#include "fornani/entities/item/types/ApparelItem.hpp"
#include "fornani/entities/item/types/GizmoItem.hpp"
#include "fornani/entities/item/types/KeyItem.hpp"

#include <memory>
#include <vector>

namespace fornani::player {

class Inventory {
  public:
	/// @brief Adds item to inventory.
	/// @returns the item's integer ID.
	int add_item(dj::Json& source, std::string_view label, item::ItemType type);
	void remove_item(int item_id, int amount);
	void reveal_item(int item_id);
	[[nodiscard]] bool has_item(int id) const;
	[[nodiscard]] bool has_item(std::string_view label) const;
	std::vector<std::unique_ptr<item::ApparelItem>> const& apparel_view() const { return m_apparel; };
	std::vector<std::unique_ptr<item::KeyItem>> const& key_items_view() const { return m_key_items; };
	std::vector<std::unique_ptr<item::GizmoItem>> const& gizmo_items_view() const { return m_gizmo_items; };
	item::Item& item_view(int id) const&;

  private:
	std::vector<std::unique_ptr<item::ApparelItem>> m_apparel{};
	std::vector<std::unique_ptr<item::KeyItem>> m_key_items{};
	std::vector<std::unique_ptr<item::GizmoItem>> m_gizmo_items{};
};

} // namespace fornani::player
