
#include "fornani/entities/npc/Vendor.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::npc {

void Vendor::generate_inventory(automa::ServiceProvider& svc) {
	inventory = {};
	if (rare_items.empty() || common_items.empty() || uncommon_items.empty()) { return; }
	auto const inventory_seed = random::random_range(0, 10000, random::get_vendor_seed());
	NANI_LOG_DEBUG(m_logger, "Inventory Seed: {}", inventory_seed);
	auto const selection_seed = random::random_range(10000, 20000, random::get_vendor_seed());
	NANI_LOG_DEBUG(m_logger, "Selection Seed: {}", selection_seed);
	for (auto const& gfi : guaranteed_finite_items) { inventory.add_item(svc.data.item, gfi); }
	for (auto i{0}; i < m_stock_size; ++i) {
		auto const rand = random::random_range(0, 1000, selection_seed + i);
		auto const selection = random::random_range(0, 1000, inventory_seed + i);
		if (rand < 6) {
			auto const choice = random::random_range(0, static_cast<int>(rare_items.size() - 1), selection);
			inventory.add_item(svc.data.item, rare_items.at(choice));
		} else if (rand < 100) {
			auto const choice = random::random_range(0, static_cast<int>(uncommon_items.size() - 1), selection);
			inventory.add_item(svc.data.item, uncommon_items.at(choice));
		} else {
			auto choice = random::random_range(0, static_cast<int>(common_items.size() - 1), selection);
			// reroll three times if we get a duplicate common item
			auto breakout = 0;
			while (inventory.has_item(common_items.at(choice)) && breakout < 3) {
				auto const reroll_seed = selection + breakout + 1;
				choice = random::random_range(0, static_cast<int>(common_items.size() - 1), reroll_seed);
				++breakout;
			}
			if (inventory.get_quantity(common_items.at(choice)) > 2) { continue; } // don't allow more than 3
			inventory.add_item(svc.data.item, common_items.at(choice));
		}
	}
}

} // namespace fornani::npc
