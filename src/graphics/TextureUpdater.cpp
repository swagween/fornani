#include "TextureUpdater.hpp"

#include <SFML/Graphics.hpp>
#include "../setup/ServiceLocator.hpp"

namespace flfx {

void TextureUpdater::load_pixel_map(sf::Texture& map_texture) {
	map.clear();
	map_colors.clear();
	auto map_data = map_texture.copyToImage();
	auto map_image_data = map_data.getPixelsPtr();
	int width = map_data.getSize().x;
	int height = map_data.getSize().y;
	int total_array_size = width * height * 4;
	for (int i = 0; i < total_array_size; ++i) {
		sf::Color current_pixel{};
		if (i % 4 == 0) {
			uint8_t r = map_image_data[i];
			uint8_t g = map_image_data[i + 1];
			uint8_t b = map_image_data[i + 2];
			uint8_t a = map_image_data[i + 3];
			current_pixel = sf::Color{r, g, b, a};
			map_colors.push_back(current_pixel);
		}
		map.push_back(i);
	}
}
void TextureUpdater::load_palette(sf::Texture& palette_texture) {
	palette.clear();
	palette_colors.clear();
	auto palette_data = palette_texture.copyToImage();
	auto palette_image_data = palette_data.getPixelsPtr();
	int width = palette_data.getSize().x;
	int height = palette_data.getSize().y;
	int total_array_size = width * height * 4;
	for (int i = 0; i < total_array_size; ++i) {
		sf::Color current_pixel{};
		if (i % 4 == 0) {
			uint8_t r = palette_image_data[i];
			uint8_t g = palette_image_data[i + 1];
			uint8_t b = palette_image_data[i + 2];
			uint8_t a = palette_image_data[i + 3];
			current_pixel = sf::Color{r, g, b, a};
			palette_colors.push_back(current_pixel);
		}
		palette.push_back(i);
	}
}
void TextureUpdater::update_texture(sf::Texture& texture) {

	// set image data with current texture
	auto texture_data = texture.copyToImage();
	auto image_data = texture_data.getPixelsPtr();
	int width = texture_data.getSize().x;
	int height = texture_data.getSize().y;
	int total_array_size = width * height * 4;

	//iterate over passed texture
	for (int i = 0; i < total_array_size; ++i) {

		// fill image
		sf::Uint8 index = i;

		sf::Color current_pixel{};
		int to_index{};

		//next pixel
		if (i % 4 == 0) {
			current_pixel = {(sf::Uint8)(image_data[i]), (sf::Uint8)(image_data[i + 1]), (sf::Uint8)(image_data[i + 2]), (sf::Uint8)(image_data[i + 3])};

			// find this pixels home index
			for (int j = 0; j < map_colors.size(); ++j) {
				// found base pixel to map, i is where we need to check in the new palette
					sf::Color map_check = map_colors.at(j);
					if (map_check == current_pixel) { to_index = j; }
			}
			// found, it's to_index
			// get the color at to_index from our new palette
			sf::Color to_color = palette_colors.at(to_index);
			image.push_back(to_color.r);
			image.push_back(to_color.g);
			image.push_back(to_color.b);
			image.push_back(to_color.a);
		}
	}

	// overwrite the texture
	texture.update(image.data());
	image.clear();
}

void flfx::TextureUpdater::debug_render(sf::RenderWindow& win, sf::Vector2<float>& campos) {
	debug.setSize({32.f, 32.f});
	if (!map_colors.empty()) {
		debug.setFillColor(map_colors.at(0));
		debug.setPosition(0, 0);
		win.draw(debug);
	}
	if (!palette_colors.empty()) {
		debug.setFillColor(palette_colors.at(0));
		debug.setPosition(32.f, 0);
		win.draw(debug);
	}
}
} // namespace flfx
