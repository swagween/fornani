
#include "fornani/entities/player/Inventory.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

int Inventory::add_item(dj::Json& source, std::string_view label, item::ItemType type) {
	switch (type) {
	case item::ItemType::apparel:
		if (!has_item(label)) { m_apparel.push_back(std::make_unique<item::ApparelItem>(source, label)); }
		return m_apparel.back()->get_id();
	case item::ItemType::key:
		if (!has_item(label)) {
			m_key_items.push_back(std::make_unique<item::KeyItem>(source, label));
			return m_key_items.back()->get_id();
		}
	}
	return 0;
}

void Inventory::remove_item(int item_id, int amount) {}

void Inventory::reveal_item(int item_id) {}

bool Inventory::has_item(int id) const {
	for (auto& item : m_apparel) {
		if (item->get_id() == id) { return true; }
	}
	for (auto& item : m_key_items) {
		if (item->get_id() == id) { return true; }
	}
	return false;
}
bool Inventory::has_item(std::string_view label) const {
	for (auto& item : m_apparel) {
		if (item->get_label() == label) { return true; }
	}
	for (auto& item : m_key_items) {
		if (item->get_label() == label) { return true; }
	}
	return false;
}

item::Item& Inventory::item_view(int id) const& {
	for (auto const& item : m_key_items) {
		if (item->get_id() == id) { return *item; }
	}
	return *m_key_items.back();
}

} // namespace fornani::player
