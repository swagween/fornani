#include "fornani/entities/npc/Vendor.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include <iostream>

namespace fornani::npc {

void Vendor::generate_inventory(automa::ServiceProvider& svc) {
	inventory.clear();
	if (rare_items.empty() || common_items.empty() || uncommon_items.empty()) {
		return;
	}
	auto inventory_seed = svc.random.random_range(0, 10000, svc.random.get_vendor_seed());
	std::cout << "Vendor Seed: " << svc.random.get_vendor_seed() << "\n";
	std::cout << "Inventory Seed: " << inventory_seed << "\n";
	for (auto i{0}; i < stock_size; ++i) {
		auto rand = svc.random.random_range(0, 1000);
		auto selection = svc.random.random_range(0, 1000, inventory_seed + i);
		std::cout << "Choices: " << selection << "\n";
		if (rand < 2) {
			auto choice = svc.random.random_range(0, static_cast<int>(rare_items.size() - 1), selection);
			inventory.add_item(svc, rare_items.at(choice), 1);
		} else if (rand < 50) {
			auto choice = svc.random.random_range(0, static_cast<int>(uncommon_items.size() - 1), selection);
			inventory.add_item(svc, uncommon_items.at(choice), 1);
		} else {
			auto choice = svc.random.random_range(0, static_cast<int>(common_items.size() - 1), selection);
			inventory.add_item(svc, common_items.at(choice), 1);
		}
	}
}

} // namespace npc
