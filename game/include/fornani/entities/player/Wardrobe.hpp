
#pragma once
#include "fornani/utils/BitFlags.hpp"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string_view>

#include "fornani/utils/Logger.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace flfx {
class TextureUpdater;
}

namespace player {

enum class ApparelType { shirt, pants, hairstyle, jacket, headgear, END };

class Wardrobe {
  public:
	Wardrobe();
	void set_palette(sf::Texture& tex);
	void change_outfit(std::vector<std::pair<sf::Vector2<unsigned int>, sf::Color>> replacement);
	void update(flfx::TextureUpdater& updater);
	void equip(ApparelType type, int variant);
	void unequip(ApparelType type);
	int get_variant(ApparelType type);
  private:
	std::unordered_map<ApparelType, int> apparel{};
	sf::Texture palette{};

	fornani::Logger m_logger { "entities" };
};

} // namespace player
