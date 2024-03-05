
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace flfx {

class TextureUpdater {

  public:
	void load_pixel_map(sf::Texture& map);
	void load_palette(sf::Texture& palette_texture);
	void update_texture(sf::Texture& texture);

	void debug_render(sf::RenderWindow& win, sf::Vector2<float>& campos);

  private:
	std::vector<sf::Uint8> image{};
	std::vector<sf::Uint8> palette{};
	std::vector<sf::Uint8> map{};

	std::vector<sf::Color> palette_colors{};
	std::vector<sf::Color> map_colors{};

	sf::RectangleShape debug{};
};
} // namespace flfx
