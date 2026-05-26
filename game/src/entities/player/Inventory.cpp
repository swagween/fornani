
#include "fornani/entities/player/Inventory.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

void Inventory::add_item(dj::Json const& source, std::string_view label) {
	if (!has_item(label)) {
		m_items.add({std::make_unique<item::Item>(source, label), 1});
	} else {
		++find_item_stack(label)->quantity;
	}
	m_latest_item = label.data();
}

void Inventory::remove_item(std::string_view tag, int amount) {
	auto const& item = find_item_stack(tag);
	if (item->quantity == 1) {
		m_items.remove(*item);
	} else {
		--item->quantity;
	}
}

EquipmentStatus Inventory::equip_item(int item_id) {

	auto num_equippable_items = 1 + get_quantity("equip_slot");

	// get the item in question
	auto this_item = find_item(item_id);
	if (this_item == nullptr) { return EquipmentStatus::failure; }

	// check if it's already equipped; if so, unequip it
	for (auto& item : m_equipped_items) {
		if (item == item_id) {
			item = -1;
			this_item->set_equipped(false);
			return EquipmentStatus::unequipped;
		}
	}
	auto num_currently_equipped = 0;
	// otherwise, equip it
	for (auto& item : m_equipped_items) {
		if (item == -1 && num_currently_equipped < num_equippable_items) {
			item = item_id;
			this_item->set_equipped(true);
			return EquipmentStatus::equipped;
		} else {
			++num_currently_equipped;
		}
		if (num_currently_equipped == num_equippable_items) {
			auto to_replace = find_item(item);
			if (to_replace == nullptr) { return EquipmentStatus::failure; }
			item = item_id;
			to_replace->set_equipped(false);
			this_item->set_equipped(true);
			return EquipmentStatus::swapped;
		}
	}

	return EquipmentStatus::failure; // no more space
}

void Inventory::reveal_item(int item_id) {
	for (auto const& item : m_items) {
		if (item.item->get_id() == item_id) { item.item->reveal(); }
	}
}

void Inventory::build_item(dj::Json const& product) {
	auto const& recipe = product["build"]["recipe"];
	for (auto const& ingredient : recipe.as_array()) { remove_item(ingredient.as_string(), 1); }
}

bool Inventory::has_item(int id) const {
	for (auto& item : m_items) {
		if (item.item->get_id() == id) { return true; }
	}
	return false;
}

bool Inventory::has_item_equipped(std::string_view id) const {
	auto item_ptr = find_item(id);
	if (item_ptr == nullptr) { return false; }
	return std::find_if(m_equipped_items.begin(), m_equipped_items.end(), [item_ptr](auto const& i) { return i == item_ptr->get_id(); }) != m_equipped_items.end();
}

bool Inventory::has_item(std::string_view label) const {
	for (auto& item : m_items) {
		if (item.item->get_label() == label) { return true; }
	}
	return false;
}

item::Item* Inventory::find_item(int id) const {
	if (auto it = std::ranges::find_if(m_items, [&](auto const& item) { return item.item->get_id() == id; }); it != m_items.end()) { return it->item.get(); }
	return nullptr;
}

item::Item* Inventory::find_item(std::string_view label) const {
	if (auto it = std::ranges::find_if(m_items, [&](auto const& item) { return item.item->get_label() == label; }); it != m_items.end()) { return it->item.get(); }
	return nullptr;
}

ItemStack* Inventory::find_item_stack(int id) {
	for (auto& stack : m_items) {
		if (!stack.item) { continue; }
		auto* ptr = stack.item.get();
		assert(ptr != nullptr); // catch early corruption
		try {
			[[maybe_unused]] auto l = ptr->get_label(); // access to detect crash
		} catch (...) { assert(false && "Corrupted Item detected"); }
		if (ptr->get_id() == id) { return &stack; }
	}
	return nullptr;
}

ItemStack* Inventory::find_item_stack(std::string_view label) {
	for (auto& stack : m_items) {
		if (!stack.item) { continue; }
		auto* ptr = stack.item.get();
		assert(ptr != nullptr); // catch early corruption
		try {
			[[maybe_unused]] auto l = ptr->get_label(); // access to detect crash
		} catch (...) { assert(false && "Corrupted Item detected"); }
		if (ptr->get_label() == label) { return &stack; }
	}
	return nullptr;
}

int Inventory::get_quantity(std::string_view label) {
	if (auto* stack = find_item_stack(label)) { return stack->quantity; }
	return 0;
}

auto Inventory::can_build(dj::Json const& product) const -> bool {
	auto const& recipe = product["build"]["recipe"];
	for (auto const& ingredient : recipe.as_array()) {
		if (!has_item(ingredient.as_string_view())) { return false; }
	}
	return true;
}

} // namespace fornani::player
