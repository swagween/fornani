
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "fornani/utils/BitFlags.hpp"

#include "fornani/io/Logger.hpp"

namespace fornani::flfx {
class TextureUpdater;
}

namespace fornani::player {

enum class ApparelType : std::uint8_t { pants, shirt, headgear, hairstyle, END };
enum class ClothingVariant : std::uint8_t { standard, red_jeans, chalcedony_tee, chalcedony_skirt, punk_hair, punk_shirt, punk_pants, ponytail };

class Wardrobe {
  public:
	Wardrobe();
	void set_palette(sf::Texture& tex);
	void change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement);
	void update(flfx::TextureUpdater& updater);
	void equip(ApparelType type, ClothingVariant variant);
	void unequip(ApparelType type);
	ClothingVariant get_variant(ApparelType type);

  private:
	std::unordered_map<ApparelType, ClothingVariant> m_apparel{};
	sf::Texture m_palette{};

	io::Logger m_logger{"entities"};
};

} // namespace fornani::player
