
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace flfx {

class TextureUpdater {

  public:
	void load_pixel_map(sf::Texture& map);
	void switch_to_palette(sf::Texture& palette_texture);
	void load_base_texture(sf::Texture& base);
	void load_palette(sf::Texture& palette_texture);
	void update_texture(sf::Texture& texture);

	void debug_render(sf::RenderWindow& win, sf::Vector2<float>& campos);

	sf::Texture& get_dynamic_texture();

  private:
	std::vector<std::uint8_t> image{};
	std::vector<std::uint8_t> palette{};
	std::vector<std::uint8_t> map{};

	std::vector<sf::Color> palette_colors{};
	std::vector<sf::Color> map_colors{};

	sf::RectangleShape debug{};

	sf::Texture base_texture{};
	sf::Texture dynamic_texture{};
};
} // namespace flfx
