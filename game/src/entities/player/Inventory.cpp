
#include "fornani/entities/player/Inventory.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

int Inventory::add_item(dj::Json& source, std::string_view label) {
	if (!has_item(label)) {
		m_items.push_back(std::make_unique<item::Item>(source, label));
		return m_items.back()->get_id();
	}

	return 0;
}

void Inventory::remove_item(int item_id, int amount) {}

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
