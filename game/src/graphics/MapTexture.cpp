
#include "fornani/graphics/MapTexture.hpp"
#include <ranges>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapTexture::MapTexture(automa::ServiceProvider& svc) : m_border_color{colors::pioneer_red}, m_tile_color{colors::pioneer_dark_red}, m_scale{4.f} {
	m_tile_box.setFillColor(m_tile_color);
	m_tile_box.setSize({m_scale, m_scale});
}

void MapTexture::bake(dj::Json const& in) {
	auto const& in_meta = in["meta"];
	auto const& in_tile = in["tile"];
	m_room_id = in_meta["room_id"].as<int>();

	m_global_offset = sf::Vector2i{in_meta["metagrid"][0].as<int>(), in_meta["metagrid"][1].as<int>()} * constants::i_chunk_size * static_cast<int>(m_scale);
	auto map_dim = sf::Vector2f{in_meta["dimensions"][0].as<float>(), in_meta["dimensions"][1].as<float>()};
	m_map_dimensions = map_dim * m_scale;

	auto u_scale = static_cast<unsigned int>(m_scale);
	for (auto [i, layer] : std::views::enumerate(m_layers)) {

		auto const lores = static_cast<Resolution>(i) == Resolution::low;
		auto const medres = static_cast<Resolution>(i) == Resolution::medium;
		auto const hires = static_cast<Resolution>(i) == Resolution::high;
		auto const low_resolution_scale = 4;
		auto const med_resolution_scale = 2;

		auto u_dimensions = sf::Vector2u{map_dim};
		if (!layer.center_texture.resize(u_dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
		layer.center_texture.clear(sf::Color::Transparent);
		if (!layer.border_texture.resize(u_dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
		layer.border_texture.clear(sf::Color::Transparent);

		for (auto [j, tile] : std::views::enumerate(in["tile"]["layers"][in["tile"]["middleground"].as<int>()].as_array())) {
			if (lores) {
				if (j % low_resolution_scale == 0 || j % (u_dimensions.x * low_resolution_scale) > u_dimensions.x) { continue; }
			}
			if (medres) {
				if (j % med_resolution_scale == 0 || j % (u_dimensions.x * med_resolution_scale) > u_dimensions.x) { continue; }
			}
			auto value = tile.as<int>();
			auto type = world::get_type_by_value(value);
			auto dont_draw = type == world::TileType::solid || type == world::TileType::breakable || type == world::TileType::spawner;
			auto position = sf::Vector2f{static_cast<float>(j % u_dimensions.x), static_cast<float>(j / u_dimensions.x)};
			if (!dont_draw) {
				m_tile_box.setFillColor(m_tile_color);
				m_tile_box.setPosition(position * m_scale);
				auto draw_plat = type == world::TileType::platform && hires;
				draw_plat ? m_tile_box.setFillColor(colors::pioneer_dark_red) : m_tile_box.setFillColor(colors::pioneer_dark_red);
				auto res_scale = lores ? static_cast<float>(low_resolution_scale) : medres ? static_cast<float>(med_resolution_scale) : 1.f;
				draw_plat ? m_tile_box.setScale({res_scale, res_scale * 0.75f}) : m_tile_box.setScale({res_scale, res_scale});
				draw_plat ? m_tile_box.setOrigin({0.0f, -0.25f * res_scale * m_scale}) : m_tile_box.setOrigin({});
				layer.center_texture.draw(m_tile_box);
				m_tile_box.setFillColor(m_border_color);
				layer.border_texture.draw(m_tile_box);
			}
		}
		layer.border_texture.display();
		layer.center_texture.display();
	}
}

void MapTexture::bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current, bool undiscovered) {
	m_room_id = room;
	map.unserialize(svc, room, true);
	if (!map.is_minimap()) {
		m_ignore = true;
		return;
	}
	m_global_offset = map.metagrid_coordinates * constants::i_chunk_size * static_cast<int>(m_scale);
	m_map_dimensions = static_cast<sf::Vector2f>(map.dimensions) * m_scale;

	auto u_scale = static_cast<unsigned int>(m_scale);
	auto const& middleground = map.get_middleground();
	auto const& obscuring = map.get_obscuring_layer();
	for (auto [i, layer] : std::views::enumerate(m_layers)) {
		if (!layer.center_texture.resize(map.dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
		layer.center_texture.clear(sf::Color::Transparent);
		if (!layer.border_texture.resize(map.dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
		layer.border_texture.clear(sf::Color::Transparent);

		for (auto& cell : middleground->grid.cells) {
			auto obscured = obscuring->grid.cells.at(cell.one_d_index).is_occupied() && map.has_obscuring_layer(); // not sure if i want to use this yet
			if (!cell.is_occupied() || cell.is_platform() || cell.is_minimap_drawable()) {
				m_tile_box.setFillColor(m_tile_color);
				m_tile_box.setPosition(cell.f_scaled_position() * m_scale);
				auto draw_plat = cell.is_platform() && static_cast<Resolution>(i) == Resolution::high;
				draw_plat ? m_tile_box.setFillColor(colors::pioneer_dark_red) : m_tile_box.setFillColor(colors::pioneer_dark_red);
				draw_plat ? m_tile_box.setScale({1.f, 0.75f}) : m_tile_box.setScale({1.f, 1.f});
				draw_plat ? m_tile_box.setOrigin({0.0f, -0.25f * m_scale}) : m_tile_box.setOrigin({});
				layer.center_texture.draw(m_tile_box);
				m_tile_box.setFillColor(m_border_color);
				layer.border_texture.draw(m_tile_box);
			}
		}
		layer.border_texture.display();
		layer.center_texture.display();
	}
}

sf::RenderTexture& MapTexture::get(bool border) { return border ? current_layer().border_texture : current_layer().center_texture; }

sf::Vector2f MapTexture::get_position() const { return sf::Vector2f(static_cast<float>(m_global_offset.x), static_cast<float>(m_global_offset.y)); }

sf::Vector2f MapTexture::get_dimensions() const { return m_map_dimensions; }

} // namespace fornani::gui
