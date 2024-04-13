#include "Catalog.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"

namespace player {
void Catalog::update(automa::ServiceProvider& svc) {
	categories.inventory.update(svc);
	categories.abilities.update(svc);
}
void Catalog::add_item(automa::ServiceProvider& svc, int item_id, int amount) { categories.inventory.add_item(svc, item_id, amount); }

} // namespace player
