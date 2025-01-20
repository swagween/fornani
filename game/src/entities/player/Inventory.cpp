#include "fornani/entities/player/Inventory.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace player {

Inventory::Inventory() {}

void Inventory::update(automa::ServiceProvider& svc) {
	int index{};
	for (auto& item : items) {
		item.update(svc, index, items_per_row, ui_offset);
		++index;
	}
}

void Inventory::add_item(automa::ServiceProvider& svc, int item_id, int amount) {
	if (item_id == 0) { return; }
	bool found{};
	for (auto& item : items) {
		if (item.get_id() == item_id) {
			item.add_item(amount);
			found = true;
		}
	}
	if (!found) {
		items.push_back(item::Item(svc, svc.tables.item_labels.at(item_id)));
		items.back().set_id(item_id);
		items.back().add_item(amount);
		svc.stats.player.items_collected.update();
	}
	update(svc);
	if (svc.in_game()) { svc.soundboard.flags.item.set(audio::Item::get); }
	push_sellables();
}

void Inventory::remove_item(automa::ServiceProvider& svc, int item_id, int amount) {
	for (auto& item : items) {
		if (item.get_id() == item_id) {
			item.subtract_item(amount);
		}
	}
	std::erase_if(items, [](auto const& i) { return i.depleted(); });
	push_sellables();
}

void Inventory::reveal_item(int item_id) {
	for (auto& item : items) {
		if (item.get_id() == item_id) { item.reveal(); }
	}
}

void Inventory::push_sellables() {
	sellable_items.clear();
	auto index{0};
	for (auto& item : items) {
		if (item.sellable()) { sellable_items.push_back(index); }
		++index;
	}
}

item::Item& Inventory::get_item(int id) {
	for (auto& item : items) {
		if (item.get_id() == id) { return item; }
	}
	return items.at(0);
}

item::Item& Inventory::get_item_at_index(int index) { return items.at(index); }

void Inventory::clear() {
	items.clear();
	sellable_items.clear();
}

bool Inventory::has_item(int id) const {
	auto ret{false};
	for (auto& item : items) {
		if (item.get_id() == id) { ret = true; }
	}
	return ret;
}

} // namespace player
