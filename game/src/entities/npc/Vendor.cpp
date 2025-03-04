#include "fornani/entities/npc/Vendor.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::npc {

void Vendor::generate_inventory(automa::ServiceProvider& svc) {
	inventory.clear();
	if (rare_items.empty() || common_items.empty() || uncommon_items.empty()) { return; }
	auto const inventory_seed = util::Random::random_range(0, 10000, util::Random::get_vendor_seed());
	for (auto i{0}; i < stock_size; ++i) {
		auto const rand = util::Random::random_range(0, 1000);
		auto const selection = util::Random::random_range(0, 1000, inventory_seed + i);
		if (rand < 2) {
			auto const choice = util::Random::random_range(0, static_cast<int>(rare_items.size() - 1), selection);
			inventory.add_item(svc, rare_items.at(choice), 1);
		} else if (rand < 50) {
			auto const choice = util::Random::random_range(0, static_cast<int>(uncommon_items.size() - 1), selection);
			inventory.add_item(svc, uncommon_items.at(choice), 1);
		} else {
			auto const choice = util::Random::random_range(0, static_cast<int>(common_items.size() - 1), selection);
			inventory.add_item(svc, common_items.at(choice), 1);
		}
	}
}

} // namespace fornani::npc
