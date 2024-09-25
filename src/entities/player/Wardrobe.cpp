#include "Wardrobe.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"

namespace player {

Wardrobe::Wardrobe() {
	for (int i{}; i < static_cast<int>(ApparelType::END); ++i) { apparel.insert({static_cast<ApparelType>(i), 0}); }
}

void Wardrobe::equip(ApparelType type, int variant) {
	if (!apparel.contains(type)) { return; }
	apparel.at(type) = variant;
}

void Wardrobe::unequip(ApparelType type) {
	if (!apparel.contains(type)) { return; }
	apparel.at(type) = 0;
}

int Wardrobe::get_variant(ApparelType type) {
	if (!apparel.contains(type)) { return 0; }
	return apparel.at(type);
}

} // namespace player
