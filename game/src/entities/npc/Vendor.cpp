#include "fornani/entities/npc/Vendor.hpp"
#include <iostream>
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::npc {

void Vendor::generate_inventory(automa::ServiceProvider& svc) {
	// TODO: Remove iostream from here.
	inventory.clear();
	if (rare_items.empty() || common_items.empty() || uncommon_items.empty()) { return; }
	auto const inventory_seed = util::random::random_range(0, 10000, util::random::get_vendor_seed());
	std::cout << "Vendor Seed: " << util::random::get_vendor_seed() << "\n";
	std::cout << "Inventory Seed: " << inventory_seed << "\n";
	for (auto i{0}; i < stock_size; ++i) {
		auto const rand = util::random::random_range(0, 1000);
		auto const selection = util::random::random_range(0, 1000, inventory_seed + i);
		std::cout << "Choices: " << selection << "\n";
		if (rand < 2) {
			auto const choice = util::random::random_range(0, static_cast<int>(rare_items.size() - 1), selection);
			inventory.add_item(svc, rare_items.at(choice), 1);
		} else if (rand < 50) {
			auto const choice = util::random::random_range(0, static_cast<int>(uncommon_items.size() - 1), selection);
			inventory.add_item(svc, uncommon_items.at(choice), 1);
		} else {
			auto const choice = util::random::random_range(0, static_cast<int>(common_items.size() - 1), selection);
			inventory.add_item(svc, common_items.at(choice), 1);
		}
	}
}

} // namespace fornani::npc
