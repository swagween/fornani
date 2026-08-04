
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <vector>

namespace fornani::graphics {

class TextureUpdater {

  public:
	void load_pixel_map(sf::Texture& map);
	void switch_to_palette(sf::Texture& palette_texture);
	void load_base_texture(sf::Texture& base);
	void load_palette(sf::Texture& palette_texture);
	void update_texture();

	void debug_render(sf::RenderWindow& win, sf::Vector2f& campos);

	sf::Texture& get_dynamic_texture();

  private:
	std::unordered_map<std::uint32_t, std::size_t> m_color_to_index;

	std::vector<sf::Color> m_palette_colors{};
	std::vector<sf::Color> m_map_colors{};

	sf::RectangleShape debug{};

	sf::Vector2u m_size{};
	std::vector<std::uint8_t> m_palette_indices;
	std::vector<std::uint8_t> m_dynamic_pixels;
	sf::Texture m_dynamic_texture;

	io::Logger m_logger{"TextureUpdater"};
};

} // namespace fornani::graphics
