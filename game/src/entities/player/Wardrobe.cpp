#include "fornani/entities/player/Wardrobe.hpp"
#include "fornani/graphics/TextureUpdater.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

Wardrobe::Wardrobe() {
	for (int i{}; i < static_cast<int>(ApparelType::END); ++i) { m_apparel.insert({static_cast<ApparelType>(i), ClothingVariant::standard}); }
}

void Wardrobe::set_palette(sf::Texture& tex) { m_palette = tex; }

void Wardrobe::change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement) {
	sf::Image image = m_palette.copyToImage();
	for (auto& pixel : replacement) { image.setPixel(pixel.first, pixel.second); }
	if (!m_palette.loadFromImage(image)) { NANI_LOG_WARN(m_logger, "Outfit palette could not be loaded."); }
}

void Wardrobe::update(flfx::TextureUpdater& updater) {
	switch (get_variant(ApparelType::pants)) {
	case ClothingVariant::standard: change_outfit({{{2, 0}, sf::Color{76, 176, 199}}, {{3, 0}, sf::Color{55, 151, 174}}}); break;
	case ClothingVariant::red_jeans: change_outfit({{{2, 0}, sf::Color{215, 0, 75}}, {{3, 0}, sf::Color{160, 18, 63}}}); break;
	case ClothingVariant::chalcedony_skirt: change_outfit({{{2, 0}, sf::Color{229, 234, 160}}, {{3, 0}, sf::Color{196, 213, 119}}}); break;
	case ClothingVariant::punk_pants: change_outfit({{{2, 0}, sf::Color{33, 34, 30}}, {{3, 0}, sf::Color{20, 19, 16}}}); break;
	default: break;
	}
	switch (get_variant(ApparelType::hairstyle)) {
	case ClothingVariant::standard: change_outfit({{{0, 1}, sf::Color{255, 255, 255}}, {{2, 1}, sf::Color{156, 142, 212}}}); break;
	case ClothingVariant::punk_hair: change_outfit({{{0, 1}, sf::Color{131, 52, 227}}, {{2, 1}, sf::Color{118, 114, 249}}}); break;
	default: break;
	}
	switch (get_variant(ApparelType::shirt)) {
	case ClothingVariant::standard: change_outfit({{{3, 2}, sf::Color{24, 34, 66}}}); break;
	case ClothingVariant::chalcedony_tee: change_outfit({{{3, 2}, sf::Color{132, 113, 250}}}); break;
	case ClothingVariant::punk_shirt: change_outfit({{{3, 2}, sf::Color{194, 217, 211}}}); break;
	default: break;
	}
	updater.switch_to_palette(m_palette);
}

void Wardrobe::equip(ApparelType type, ClothingVariant variant) {
	if (!m_apparel.contains(type)) { return; }
	m_apparel.at(type) = variant;
}

void Wardrobe::unequip(ApparelType type) {
	if (!m_apparel.contains(type)) { return; }
	m_apparel.at(type) = ClothingVariant::standard;
}

ClothingVariant Wardrobe::get_variant(ApparelType type) {
	if (!m_apparel.contains(type)) { return ClothingVariant::standard; }
	return m_apparel.at(type);
}

} // namespace fornani::player
