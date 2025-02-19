#include "fornani/entities/player/Catalog.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/gui/Console.hpp"

namespace fornani::player {
void Catalog::update(automa::ServiceProvider& svc) {
	categories.inventory.update(svc);
	categories.abilities.update(svc);
}
void Catalog::add_item(automa::ServiceProvider& svc, int item_id, int amount) { categories.inventory.add_item(svc, item_id, amount); }

void Catalog::remove_item(automa::ServiceProvider& svc, int item_id, int amount) { categories.inventory.remove_item(svc, item_id, amount); }

void Catalog::equip_item(automa::ServiceProvider& svc, ApparelType type, int item_id) {
	auto variant = item_id - 80;
	for (auto& item : categories.inventory.items) {
		if (!item.equippable()) { continue; }
		if (item.is_equipped() && item.get_apparel_type() == type) {
			categories.wardrobe.unequip(type);
			item.toggle_equip();
		}
	}
	categories.wardrobe.equip(type, variant);
}

void Catalog::unequip_item(ApparelType type) { categories.wardrobe.unequip(type); }

} // namespace player
