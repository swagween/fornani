#include "fornani/graphics/TextureUpdater.hpp"
#include <SFML/Graphics.hpp>

namespace fornani::graphics {

void TextureUpdater::load_pixel_map(sf::Texture& map_texture) {
	auto const image = map_texture.copyToImage();
	auto const* pixels = image.getPixelsPtr();

	auto const size = image.getSize();
	auto const pixel_count = static_cast<std::size_t>(size.x) * size.y;

	m_color_to_index.clear();
	m_color_to_index.reserve(pixel_count);

	for (std::size_t pixel{}; pixel < pixel_count; ++pixel) {
		auto const i = pixel * 4;
		auto const key = (static_cast<std::uint32_t>(pixels[i + 0]) << 24) | (static_cast<std::uint32_t>(pixels[i + 1]) << 16) | (static_cast<std::uint32_t>(pixels[i + 2]) << 8) | static_cast<std::uint32_t>(pixels[i + 3]);
		m_color_to_index.emplace(key, pixel);
	}
}

void TextureUpdater::switch_to_palette(sf::Texture& palette_texture) {
	load_palette(palette_texture);

	update_texture();

	m_dynamic_texture.update(m_dynamic_pixels.data());
}

void TextureUpdater::load_base_texture(sf::Texture& base) {

	auto image = base.copyToImage();
	m_size = image.getSize();
	auto const pixel_count = static_cast<std::size_t>(m_size.x) * m_size.y;
	auto const* pixels = image.getPixelsPtr();
	m_palette_indices.resize(pixel_count);

	for (std::size_t pixel{}; pixel < pixel_count; ++pixel) {
		auto const i = pixel * 4;
		if (pixels[i + 3] == 0) {
			m_palette_indices[pixel] = 0xFF;
			continue;
		}

		auto const key = (static_cast<std::uint32_t>(pixels[i + 0]) << 24) | (static_cast<std::uint32_t>(pixels[i + 1]) << 16) | (static_cast<std::uint32_t>(pixels[i + 2]) << 8) | static_cast<std::uint32_t>(pixels[i + 3]);
		auto const it = m_color_to_index.find(key);
		if (it == m_color_to_index.end()) {
			// NANI_LOG_ERROR(m_logger, "Unknown palette color in base texture: ({}, {}, {}, {})", pixels[i + 0], pixels[i + 1], pixels[i + 2], pixels[i + 3]);
			continue;
		}

		m_palette_indices[pixel] = static_cast<std::uint8_t>(it->second);
	}

	auto const count = m_size.x * m_size.y * 4;
	m_dynamic_pixels.resize(count);
	if (!m_dynamic_texture.loadFromImage(image)) { NANI_LOG_ERROR(m_logger, "Failed to load image in load_base_texture!"); }
}

void TextureUpdater::load_palette(sf::Texture& palette_texture) {
	auto const image = palette_texture.copyToImage();
	auto const* pixels = image.getPixelsPtr();

	auto const pixel_count = static_cast<std::size_t>(image.getSize().x) * image.getSize().y;

	m_palette_colors.resize(pixel_count);

	for (std::size_t pixel{}; pixel < pixel_count; ++pixel) {

		auto const i = pixel * 4;

		m_palette_colors[pixel] = sf::Color{pixels[i + 0], pixels[i + 1], pixels[i + 2], pixels[i + 3]};
	}
}

void TextureUpdater::update_texture() {
	for (std::size_t pixel{}; pixel < m_palette_indices.size(); ++pixel) {
		auto const index = m_palette_indices[pixel];
		auto const i = pixel * 4;

		if (index == 0xFF) {
			m_dynamic_pixels[i + 0] = 0;
			m_dynamic_pixels[i + 1] = 0;
			m_dynamic_pixels[i + 2] = 0;
			m_dynamic_pixels[i + 3] = 0;
			continue;
		}

		auto const& color = m_palette_colors[index];

		m_dynamic_pixels[i + 0] = color.r;
		m_dynamic_pixels[i + 1] = color.g;
		m_dynamic_pixels[i + 2] = color.b;
		m_dynamic_pixels[i + 3] = color.a;
	}
}

void graphics::TextureUpdater::debug_render(sf::RenderWindow& win, sf::Vector2f& campos) {
	debug.setSize({8.f, 8.f});
	int i{};
	for (auto& color : m_map_colors) {
		debug.setFillColor(color);
		debug.setPosition({i * 8.f, 0.f});
		win.draw(debug);
		++i;
	}
	i = 0;
	for (auto& color : m_palette_colors) {
		debug.setFillColor(color);
		debug.setPosition({i * 8.f, 10.f});
		win.draw(debug);
		++i;
	}
}

sf::Texture& TextureUpdater::get_dynamic_texture() { return m_dynamic_texture; }

} // namespace fornani::graphics
