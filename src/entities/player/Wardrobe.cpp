#include "Wardrobe.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"
#include "../../graphics/TextureUpdater.hpp"

namespace player {

Wardrobe::Wardrobe() {
	for (int i{}; i < static_cast<int>(ApparelType::END); ++i) { apparel.insert({static_cast<ApparelType>(i), 0}); }
}

void Wardrobe::set_palette(sf::Texture& tex) { palette = tex; }

void Wardrobe::change_outfit(std::vector<std::pair<sf::Vector2<int>, sf::Color>> replacement) {
	sf::Image image = palette.copyToImage();
	for (auto& pixel : replacement) { image.setPixel(pixel.first.x, pixel.first.y, pixel.second); }
	palette.loadFromImage(image);
}

void Wardrobe::update(flfx::TextureUpdater& updater) { 
	switch (get_variant(player::ApparelType::pants)) {
	case 0: change_outfit({{{2, 0}, sf::Color{76, 176, 199}}, {{3, 0}, sf::Color{55, 151, 174}}}); break;
	case 1: change_outfit({{{2, 0}, sf::Color{215, 0, 75}}, {{3, 0}, sf::Color{160, 18, 63}}}); break;
	}
	switch (get_variant(player::ApparelType::hairstyle)) {
	}
	switch (get_variant(player::ApparelType::shirt)) {
	}
	updater.switch_to_palette(palette);
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
