
#include "fornani/entities/player/Inventory.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

int Inventory::add_item(dj::Json& source, std::string_view label) {
	if (!has_item(label)) {
		m_items.push_back({std::make_unique<item::Item>(source, label), 1});
		return m_items.back().item->get_id();
	} else {
		++find_item_stack(label)->quantity;
	}
	return 0;
}

void Inventory::remove_item(int item_id, int amount) {
	std::erase_if(m_items, [item_id](auto const& i) { return i.item->get_id() == item_id; });
}

EquipmentStatus Inventory::equip_item(int item_id) {

	auto num_equippable_items = 1 + get_quantity("equip_slot");
	// get the item in question
	auto this_item = std::find_if(m_items.begin(), m_items.end(), [item_id](auto const& i) { return i.item->get_id() == item_id; });
	if (this_item == m_items.end()) { return EquipmentStatus::failure; }

	// check if it's already equipped; if so, unequip it
	for (auto& item : m_equipped_items) {
		if (item == item_id) {
			item = -1;
			this_item->item.get()->set_equipped(false);
			return EquipmentStatus::unequipped;
		}
	}
	auto num_currently_equipped = 0;
	// otherwise, equip it
	for (auto& item : m_equipped_items) {
		if (item == -1 && num_currently_equipped < num_equippable_items) {
			item = item_id;
			this_item->item.get()->set_equipped(true);
			return EquipmentStatus::equipped;
		} else {
			++num_currently_equipped;
		}
	}

	return EquipmentStatus::failure; // no more space
}

void Inventory::reveal_item(int item_id) {
	for (auto const& item : m_items) {
		if (item.item->get_id() == item_id) { item.item->reveal(); }
	}
}

bool Inventory::has_item(int id) const {
	for (auto& item : m_items) {
		if (item.item->get_id() == id) { return true; }
	}
	return false;
}

bool Inventory::has_item_equipped(int id) const {
	return std::find_if(m_equipped_items.begin(), m_equipped_items.end(), [id](auto const& i) { return i == id; }) != m_equipped_items.end();
}

bool Inventory::has_item(std::string_view label) const {
	for (auto& item : m_items) {
		if (item.item->get_label() == label) { return true; }
	}
	return false;
}

int Inventory::get_quantity(std::string_view label) { return find_item_stack(label) == nullptr ? 0 : find_item_stack(label)->quantity; }

item::Item* Inventory::find_item(int id) const {
	if (auto it = std::ranges::find_if(m_items, [&](auto const& item) { return item.item->get_id() == id; }); it != m_items.end()) { return it->item.get(); }
	return nullptr;
}

item::Item* Inventory::find_item(std::string_view label) const {
	if (auto it = std::ranges::find_if(m_items, [&](auto const& item) { return item.item->get_label() == label; }); it != m_items.end()) { return it->item.get(); }
	return nullptr;
}

ItemStack* Inventory::find_item_stack(std::string_view label) {
	if (auto it = std::ranges::find_if(m_items, [&](auto const& stack) { return stack.item->get_label() == label; }); it != m_items.end()) { return std::addressof(*it); }
	return nullptr;
}

} // namespace fornani::player
