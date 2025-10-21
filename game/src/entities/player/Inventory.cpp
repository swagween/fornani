
#include "fornani/entities/player/Inventory.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

int Inventory::add_item(dj::Json& source, std::string_view label) {
	if (!has_item(label)) {
		m_items.push_back(std::make_unique<item::Item>(source, label));
		return m_items.back()->get_id();
	}
	return 0;
}

void Inventory::remove_item(int item_id, int amount) {
	std::erase_if(m_items, [item_id](auto const& i) { return i->get_id() == item_id; });
}

bool Inventory::equip_item(int item_id) {

	// get the item in question
	auto this_item = std::find_if(m_items.begin(), m_items.end(), [item_id](auto const& i) { return i->get_id() == item_id; });
	if (this_item == m_items.end()) { return false; }

	// check if it's already equipped; if so, unequip it
	for (auto& item : m_equipped_items) {
		if (item == item_id) {
			item = -1;
			this_item->get()->set_equipped(false);
			return true;
		}
	}
	// otherwise, equip it
	for (auto& item : m_equipped_items) {
		if (item == -1) {
			item = item_id;
			this_item->get()->set_equipped(true);
			return true;
		}
	}
	return false; // no more space
}

void Inventory::reveal_item(int item_id) {
	for (auto const& item : m_items) {
		if (item->get_id() == item_id) { item->reveal(); }
	}
}

bool Inventory::has_item(int id) const {
	for (auto& item : m_items) {
		if (item->get_id() == id) { return true; }
	}
	return false;
}

bool Inventory::has_item_equipped(int id) const {
	return std::find_if(m_equipped_items.begin(), m_equipped_items.end(), [id](auto const& i) { return i == id; }) != m_equipped_items.end();
}

bool Inventory::has_item(std::string_view label) const {
	for (auto& item : m_items) {
		if (item->get_label() == label) { return true; }
	}
	return false;
}

item::Item& Inventory::item_view(int id) const& {
	for (auto const& item : m_items) {
		if (item->get_id() == id) { return *item; }
	}
	return *m_items.back();
}

} // namespace fornani::player
