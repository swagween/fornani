
#pragma once

#include <fornani/entities/item/Item.hpp>
#include <memory>
#include <vector>

namespace fornani::player {

constexpr auto num_equippable_items_v = 4;

enum class EquipmentStatus { equipped, unequipped, failure };

struct ItemStack {
	std::unique_ptr<item::Item> item{};
	int quantity{};
};

class Inventory {
  public:
	/// @brief Adds item to inventory.
	/// @returns the item's integer ID.
	int add_item(dj::Json& source, std::string_view label);
	void remove_item(int item_id, int amount);
	void reveal_item(int item_id);
	void add_equip_slot(int amount = 1) { m_open_equip_slots = std::clamp(m_open_equip_slots + amount, 0, num_equippable_items_v); }
	[[nodiscard]] EquipmentStatus equip_item(int item_id);
	[[nodiscard]] bool has_item(int id) const;
	[[nodiscard]] bool has_item_equipped(int id) const;
	[[nodiscard]] bool has_item(std::string_view label) const;
	[[nodiscard]] int get_quantity(std::string_view label);
	std::vector<ItemStack> const& items_view() const { return m_items; }
	std::array<int, num_equippable_items_v> const& equipped_items_view() const { return m_equipped_items; }
	item::Item* find_item(int id) const;
	item::Item* find_item(std::string_view label) const;
	ItemStack* find_item_stack(std::string_view label);

  private:
	std::vector<ItemStack> m_items{};
	std::array<int, num_equippable_items_v> m_equipped_items{-1, -1, -1, -1};
	int m_open_equip_slots{1};
};

} // namespace fornani::player
