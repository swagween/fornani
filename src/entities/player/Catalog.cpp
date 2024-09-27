#include "Catalog.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"

namespace player {
void Catalog::update(automa::ServiceProvider& svc) {
	categories.inventory.update(svc);
	categories.abilities.update(svc);
}
void Catalog::add_item(automa::ServiceProvider& svc, int item_id, int amount) { categories.inventory.add_item(svc, item_id, amount); }

void Catalog::remove_item(automa::ServiceProvider& svc, int item_id, int amount) { categories.inventory.remove_item(svc, item_id, amount); }

void Catalog::equip_item(automa::ServiceProvider& svc, ApparelType type, int item_id) {
	auto variant = item_id - 80;
	categories.wardrobe.equip(type, variant);
}

void Catalog::unequip_item(ApparelType type) { categories.wardrobe.unequip(type); }

} // namespace player
