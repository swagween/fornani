#include "fornani/entities/npc/Vendor.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::npc {

void Vendor::generate_inventory(automa::ServiceProvider& svc) {
	inventory = {};
	if (rare_items.empty() || common_items.empty() || uncommon_items.empty()) { return; }
	auto const inventory_seed = random::random_range(0, 10000, random::get_vendor_seed());
	for (auto const& gfi : guaranteed_finite_items) { inventory.add_item(svc.data.item, gfi); }
	for (auto i{0}; i < stock_size; ++i) {
		auto const rand = random::random_range(0, 1000);
		auto const selection = random::random_range(0, 1000, inventory_seed + i);
		// TODO: allow for different item types. Vendors may also be able to sell some key items and apparel.
		if (rand < 2) {
			auto const choice = random::random_range(0, static_cast<int>(rare_items.size() - 1), selection);
			inventory.add_item(svc.data.item, rare_items.at(choice));
		} else if (rand < 50) {
			auto const choice = random::random_range(0, static_cast<int>(uncommon_items.size() - 1), selection);
			inventory.add_item(svc.data.item, uncommon_items.at(choice));
		} else {
			auto const choice = random::random_range(0, static_cast<int>(common_items.size() - 1), selection);
			inventory.add_item(svc.data.item, common_items.at(choice));
		}
	}
}

} // namespace fornani::npc
