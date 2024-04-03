#include "Inventory.hpp"
#include "../../gui/Console.hpp"
#include "../../service/ServiceProvider.hpp"

namespace player {

Inventory::Inventory() {
	item_labels.insert({1, "bryns_notebook"});
	item_labels.insert({2, "factory_access_permit"});
	item_labels.insert({3, "bipolar_junction_transistor"});
	item_labels.insert({4, "willets_floppy_disc"});
	item_labels.insert({5, "track_zero_sensor"});
	item_labels.insert({6, "head_motor"});
	item_labels.insert({7, "gas_mask"});
	item_labels.insert({8, "firstwind_deck_key"});
	item_labels.insert({9, "woodshine_warehouse_key"});
	item_labels.insert({10, "nimbus_iii_boiler_room_key"});
}

void Inventory::add_item(automa::ServiceProvider& svc, int item_id, int amount) {
	bool found{};
	for (auto& item : items) {
		if (item.get_id() == item_id) {
			item.add_item(amount);
			found = true;
		}
	}
	if (!found) {
		items.push_back(item::Item(svc, item_labels.at(item_id)));
		items.back().set_id(item_id);
		items.back().add_item(amount);
	}
}

} // namespace player
