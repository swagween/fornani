
#include "fornani/entities/player/Wardrobe.hpp"
#include <fornani/gui/console/Console.hpp>
#include "fornani/graphics/TextureUpdater.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

Wardrobe::Wardrobe() {
	m_outfit.insert({ApparelType::hairstyle, std::make_unique<Hairstyle>()});
	m_outfit.insert({ApparelType::headgear, std::make_unique<Headgear>()});
	m_outfit.insert({ApparelType::shirt, std::make_unique<Shirt>()});
	m_outfit.insert({ApparelType::pants, std::make_unique<Pants>()});
}

void Wardrobe::set_palette(sf::Texture& tex) { m_palette = tex; }

void Wardrobe::change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement) {
	sf::Image image = m_palette.copyToImage();
	for (auto& pixel : replacement) { image.setPixel(pixel.first, pixel.second); }
	if (!m_palette.loadFromImage(image)) { NANI_LOG_WARN(m_logger, "Outfit palette could not be loaded."); }
}

void Wardrobe::update(graphics::TextureUpdater& updater) {
	switch (get_variant(ApparelType::pants)) {
	case 0: change_outfit({{{2, 0}, sf::Color{76, 176, 199}}, {{3, 0}, sf::Color{55, 151, 174}}}); break;
	case 1: change_outfit({{{2, 0}, sf::Color{215, 0, 75}}, {{3, 0}, sf::Color{160, 18, 63}}}); break;
	case 2: change_outfit({{{2, 0}, sf::Color{229, 234, 160}}, {{3, 0}, sf::Color{196, 213, 119}}}); break;
	case 3: change_outfit({{{2, 0}, sf::Color{33, 34, 30}}, {{3, 0}, sf::Color{20, 19, 16}}}); break;
	case 4: change_outfit({{{2, 0}, sf::Color{33, 34, 30}}, {{3, 0}, sf::Color{20, 19, 16}}}); break;
	default: break;
	}
	switch (get_variant(ApparelType::hairstyle)) {
	case 0: change_outfit({{{0, 1}, sf::Color{255, 255, 255}}, {{2, 1}, sf::Color{156, 142, 212}}}); break;
	case 2: change_outfit({{{0, 1}, sf::Color{131, 52, 227}}, {{2, 1}, sf::Color{118, 114, 249}}}); break;
	default: break;
	}
	switch (get_variant(ApparelType::shirt)) {
	case 0: change_outfit({{{3, 2}, sf::Color{24, 34, 66}}}); break;
	case 2: change_outfit({{{3, 2}, sf::Color{132, 113, 250}}}); break;
	case 3: change_outfit({{{3, 2}, sf::Color{194, 217, 211}}}); break;
	case 4: change_outfit({{{3, 2}, sf::Color{33, 34, 30}}, {{3, 0}, sf::Color{20, 19, 16}}}); break;
	default: break;
	}
	updater.switch_to_palette(m_palette);
}

void Wardrobe::equip(ApparelType type, int variant) {
	if (!m_outfit.contains(type)) {
		NANI_LOG_DEBUG(m_logger, "Does not contain type {}", static_cast<int>(type));
		return;
	}
	m_outfit.at(type)->set(variant);
}

void Wardrobe::unequip(ApparelType type) {
	if (!m_outfit.contains(type)) { return; }
	m_outfit.at(type)->set(0);
}

int Wardrobe::get_variant(ApparelType type) const {
	if (!m_outfit.contains(type)) { return 0; }
	return m_outfit.at(type)->get_variant();
}

std::array<int, static_cast<int>(ApparelType::END)> Wardrobe::get() {
	return std::array<int, static_cast<int>(ApparelType::END)>{m_outfit.at(ApparelType::hairstyle)->get_variant(), m_outfit.at(ApparelType::headgear)->get_variant(), m_outfit.at(ApparelType::shirt)->get_variant(),
															   m_outfit.at(ApparelType::pants)->get_variant()};
}

} // namespace fornani::player
