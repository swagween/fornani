
#pragma once

#include <fornani/entities/item/Item.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/systems/Register.hpp>
#include <memory>
#include <vector>

namespace fornani::player {

constexpr auto num_equippable_items_v = 4;

enum class EquipmentStatus { equipped, unequipped, failure };

struct ItemStack {
	std::unique_ptr<item::Item> item{};
	int quantity{};

	bool operator==(ItemStack const& other) const {
		if (!item || !other.item) { return item == other.item; } // both nullptr = equal
		return item->get_id() == other.item->get_id();			 // or get_label() if tag-based
	}
};

class Inventory {
  public:
	/// @brief Adds item to inventory.
	void add_item(dj::Json const& source, std::string_view label);
	void remove_item(std::string_view tag, int amount);
	void reveal_item(int item_id);
	void add_equip_slot(int amount = 1) { m_open_equip_slots = std::clamp(m_open_equip_slots + amount, 0, num_equippable_items_v); }
	[[nodiscard]] EquipmentStatus equip_item(int item_id);
	[[nodiscard]] bool has_item(int id) const;
	[[nodiscard]] bool has_item_equipped(std::string_view id) const;
	[[nodiscard]] bool has_item(std::string_view label) const;
	[[nodiscard]] int get_quantity(std::string_view label);
	Register<ItemStack> const& items_view() const { return m_items; }
	std::array<int, num_equippable_items_v> const& equipped_items_view() const { return m_equipped_items; }
	item::Item* find_item(int id) const;
	item::Item* find_item(std::string_view label) const;
	ItemStack* find_item_stack(int id);
	ItemStack* find_item_stack(std::string_view label);

  private:
	Register<ItemStack> m_items{};
	std::array<int, num_equippable_items_v> m_equipped_items{-1, -1, -1, -1};
	int m_open_equip_slots{1};

	io::Logger m_logger{"Inventory"};
};

} // namespace fornani::player
