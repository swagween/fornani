#include "fornani/entities/player/Catalog.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

void Catalog::update(automa::ServiceProvider& svc) {
	inventory.update(svc);
	abilities.update(svc);
}
void Catalog::add_item(automa::ServiceProvider& svc, int item_id, int amount) { inventory.add_item(svc, item_id, amount); }

void Catalog::remove_item(automa::ServiceProvider& svc, int item_id, int amount) { inventory.remove_item(svc, item_id, amount); }

void Catalog::equip_item(automa::ServiceProvider& svc, ApparelType type, int item_id) {
	auto variant = static_cast<ClothingVariant>(item_id - m_apparel_index);
	for (auto& item : inventory.items) {
		if (!item.equippable()) { continue; }
		if (item.is_equipped() && item.get_apparel_type() == type) {
			wardrobe.unequip(type);
			item.toggle_equip();
		}
	}
	wardrobe.equip(type, variant);
}

void Catalog::unequip_item(ApparelType type) { wardrobe.unequip(type); }

} // namespace fornani::player
