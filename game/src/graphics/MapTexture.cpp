#include "fornani/graphics/MapTexture.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapTexture::MapTexture(automa::ServiceProvider& svc) : border_color{svc.styles.colors.pioneer_red}, tile_color{svc.styles.colors.pioneer_dark_red}, m_scale{4.f} {
	tile_box.setFillColor(tile_color);
	tile_box.setSize({m_scale, m_scale});
}

void MapTexture::bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current, bool undiscovered) {
	room_id = room;
	map.load(svc, room, true);
	if (!map.is_minimap()) {
		ignore = true;
		return;
	}
	global_offset = map.metagrid_coordinates * svc.constants.i_chunk_size * static_cast<int>(m_scale);
	map_dimensions = static_cast<sf::Vector2<float>>(map.dimensions) * m_scale;

	auto u_scale = static_cast<unsigned int>(m_scale);
	auto const& middleground = map.get_middleground();
	auto const& obscuring = map.get_obscuring_layer();
	if (!m_center_texture.resize(map.dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_center_texture.clear(sf::Color::Transparent);
	if (!m_border_texture.resize(map.dimensions * u_scale)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_border_texture.clear(sf::Color::Transparent);

	for (auto& cell : middleground.grid.cells) {
		auto obscured{obscuring.grid.cells.at(cell.one_d_index).is_occupied() && map.has_obscuring_layer()};
		if (!cell.is_breakable()) {
			if (!cell.is_occupied() || cell.is_platform()) {
				tile_box.setFillColor(tile_color);
				tile_box.setPosition(cell.f_scaled_position() * m_scale);
				cell.is_platform() ? tile_box.setFillColor(svc.styles.colors.pioneer_dark_red) : tile_box.setFillColor(svc.styles.colors.pioneer_dark_red);
				cell.is_platform() ? tile_box.setScale({1.f, 0.75f}) : tile_box.setScale({1.f, 1.f});
				cell.is_platform() ? tile_box.setOrigin({0.0f, -0.25f * m_scale}) : tile_box.setOrigin({});
				if (!obscured) { m_center_texture.draw(tile_box); }
				tile_box.setFillColor(border_color);
				if (!obscured) { m_border_texture.draw(tile_box); }
			}
		}
	}

	m_border_texture.display();
	m_center_texture.display();
}

sf::RenderTexture& MapTexture::get(bool border) { return border ? m_border_texture : m_center_texture; }

sf::Vector2<float> MapTexture::get_position() { return sf::Vector2<float>(static_cast<float>(global_offset.x), static_cast<float>(global_offset.y)); }

sf::Vector2<float> MapTexture::get_dimensions() const { return map_dimensions; }

} // namespace fornani::gui
