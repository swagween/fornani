#include "fornani/entities/player/Wardrobe.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/graphics/TextureUpdater.hpp"

namespace fornani::player {

Wardrobe::Wardrobe() {
	for (int i{}; i < static_cast<int>(ApparelType::END); ++i) { apparel.insert({static_cast<ApparelType>(i), 0}); }
}

void Wardrobe::set_palette(sf::Texture& tex) { palette = tex; }

void Wardrobe::change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement) {
	sf::Image image = palette.copyToImage();
	for (auto& pixel : replacement) { image.setPixel(pixel.first, pixel.second); }
	if (!palette.loadFromImage(image)) { NANI_LOG_WARN(m_logger, "Outfit palette could not be loaded."); }
}

void Wardrobe::update(flfx::TextureUpdater& updater) { 
	switch (get_variant(player::ApparelType::pants)) {
	case 0: change_outfit({{{2, 0}, sf::Color{76, 176, 199}}, {{3, 0}, sf::Color{55, 151, 174}}}); break;
	case 1: change_outfit({{{2, 0}, sf::Color{215, 0, 75}}, {{3, 0}, sf::Color{160, 18, 63}}}); break;
	case 3: change_outfit({{{2, 0}, sf::Color{229, 234, 160}}, {{3, 0}, sf::Color{196, 213, 119}}}); break;
	case 6: change_outfit({{{2, 0}, sf::Color{33, 34, 30}}, {{3, 0}, sf::Color{20, 19, 16}}}); break;
	default: break;
	}
	switch (get_variant(player::ApparelType::hairstyle)) {
	case 0: change_outfit({{{0, 1}, sf::Color{255, 255, 255}}, {{2, 1}, sf::Color{156, 142, 212}}}); break;
	case 4: change_outfit({{{0, 1}, sf::Color{131, 52, 227}}, {{2, 1}, sf::Color{118, 114, 249}}}); break;
	default: break;
	}
	switch (get_variant(player::ApparelType::shirt)) {
	case 0: change_outfit({{{3, 2}, sf::Color{24, 34, 66}}}); break;
	case 2: change_outfit({{{3, 2}, sf::Color{132, 113, 250}}}); break;
	case 5: change_outfit({{{3, 2}, sf::Color{194, 217, 211}}}); break;
	default: break;
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
