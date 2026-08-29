
#include "fornani/graphics/MapTexture.hpp"
#include <ranges>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapTexture::MapTexture(automa::ServiceProvider& svc) : m_scale{4.f} {}

void MapTexture::bake(dj::Json const& in) {
	auto const& in_meta = in["meta"];
	auto const& in_tile = in["tile"];
	m_id = in_meta["room_id"].as<int>();
	m_biome = in_meta["biome"].as<int>();

	m_global_offset = sf::Vector2i{in_meta["metagrid"][0].as<int>(), in_meta["metagrid"][1].as<int>()} * constants::i_chunk_size * static_cast<int>(m_scale);
	auto map_dim = sf::Vector2f{in_meta["dimensions"][0].as<float>(), in_meta["dimensions"][1].as<float>()};
	m_map_dimensions = map_dim * m_scale;

	auto const& layers = in_tile["layers"];
	auto const middleground = in_tile["middleground"].as<int>();
	auto const& tiles = layers[middleground].as_array();
	auto const u_dimensions = sf::Vector2u{map_dim};

	auto u_scale = static_cast<unsigned int>(m_scale);
	for (auto [i, layer] : std::views::enumerate(m_layers)) {

		auto const lores = static_cast<Resolution>(i) == Resolution::low;
		auto const medres = static_cast<Resolution>(i) == Resolution::medium;
		auto const hires = static_cast<Resolution>(i) == Resolution::high;
		auto const lod = lores ? 4u : medres ? 2u : 1u;

		auto const image_width = (u_dimensions.x * u_scale) / lod;
		auto const image_height = (u_dimensions.y * u_scale) / lod;
		if (!layer.texture.resize({image_width, image_height})) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
		auto const image_size = static_cast<std::size_t>(image_width) * static_cast<std::size_t>(image_height) * 4;

		m_pixels.assign(image_size, 0);

		auto const tile_size = static_cast<unsigned>(m_scale);
		auto const stride = static_cast<std::size_t>(image_width);

		for (unsigned ty{}; ty < u_dimensions.y; ty += lod) {
			for (unsigned tx{}; tx < u_dimensions.x; tx += lod) {

				auto const j = ty * u_dimensions.x + tx;
				auto const type = world::get_type_by_value(tiles[j].as<int>());

				if (type == world::TileType::solid || type == world::TileType::breakable || type == world::TileType::spawner) { continue; }

				auto const draw_plat = type == world::TileType::platform && hires;

				auto const x0 = (tx / lod) * tile_size;
				auto const y0 = (ty / lod) * tile_size;

				auto const height = draw_plat ? tile_size / 2 : tile_size;
				auto const y_offset = draw_plat ? tile_size - height : 0;

				auto const max_x = std::min(x0 + tile_size, image_width);
				auto const max_y = std::min(y0 + y_offset + height, image_height);

				for (unsigned py = y0 + y_offset; py < max_y; ++py) {
					auto const row = static_cast<std::size_t>(py) * stride * 4;
					for (unsigned px = x0; px < max_x; ++px) { m_pixels[row + static_cast<std::size_t>(px) * 4 + 3] = 255; }
				}
			}
		}
		sf::Image image({image_width, image_height}, m_pixels.data());
		if (!layer.texture.loadFromImage(image)) { NANI_LOG_WARN(m_logger, "Failed to load image into map texture"); }
	}
}

auto MapTexture::contains(sf::Vector2f point) const -> bool {
	auto pt = point - sf::Vector2f{m_global_offset};
	if (pt.x < 0.f || pt.x > m_map_dimensions.x) { return false; }
	if (pt.y < 0.f || pt.y > m_map_dimensions.y) { return false; }
	return true;
}

sf::Texture& MapTexture::get() { return current_layer().texture; }

sf::Vector2f MapTexture::get_position() const { return sf::Vector2f(static_cast<float>(m_global_offset.x), static_cast<float>(m_global_offset.y)); }

sf::Vector2f MapTexture::get_dimensions() const { return m_map_dimensions; }

} // namespace fornani::gui
