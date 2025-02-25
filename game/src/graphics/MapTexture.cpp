#include "fornani/graphics/MapTexture.hpp"
#include <algorithm>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapTexture::MapTexture(automa::ServiceProvider& svc) : border_color{svc.styles.colors.pioneer_red}, tile_color{svc.styles.colors.pioneer_dark_red} {
	tile_box.setFillColor(tile_color);
	tile_box.setSize({1.f, 1.f});
	plat_box.setFillColor(svc.styles.colors.dark_fucshia);
	plat_box.setSize({1.f, 1.f});
	portal_box.setFillColor(svc.styles.colors.dark_fucshia);
	portal_box.setSize({1.f, 1.f});
	breakable_box.setFillColor(svc.styles.colors.dark_fucshia);
	breakable_box.setSize({1.f, 1.f});
	save_box.setFillColor(svc.styles.colors.dark_fucshia);
	save_box.setSize({1.f, 1.f});
}

void MapTexture::bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current, bool undiscovered) {
	map.load(svc, room, true);
	if (!map.is_minimap()) {
		ignore = true;
		return;
	}
	global_offset = map.metagrid_coordinates * svc.constants.i_chunk_size;
	map_dimensions = static_cast<sf::Vector2<float>>(map.dimensions);

	auto const& middleground = map.get_middleground();
	auto const& obscuring = map.get_obscuring_layer();
	if (!m_center_texture.resize(map.dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_center_texture.clear(sf::Color::Transparent);
	if (!m_border_texture.resize(map.dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_border_texture.clear(sf::Color::Transparent);

	for (auto& cell : middleground.grid.cells) {
		auto obscured{obscuring.grid.cells.at(cell.one_d_index).is_occupied() && map.has_obscuring_layer()};
		if (!cell.is_breakable()) {
			if (!cell.is_occupied()) {
				tile_box.setFillColor(tile_color);
				tile_box.setPosition(cell.f_scaled_position());
				if (!obscured) { m_center_texture.draw(tile_box); }
				tile_box.setFillColor(border_color);
				if (!obscured) { m_border_texture.draw(tile_box); }
			}
		}
	}

	for (auto& portal : map.portals) {
		portal_box.setPosition(portal.position);
		portal_box.setSize(portal.dimensions);
		m_center_texture.draw(portal_box);
	}
	for (auto& breakable : map.breakables) {
		breakable_box.setPosition(breakable.get_bounding_box().get_position());
		breakable_box.setSize(breakable.get_bounding_box().get_dimensions());
		m_center_texture.draw(breakable_box);
	}
	if (map.save_point.id > 0) {
		save_box.setPosition(sf::Vector2<float>(map.save_point.scaled_position) * svc.constants.cell_size);
		m_center_texture.draw(save_box);
	}

	m_border_texture.display();
	m_center_texture.display();
}

sf::RenderTexture& MapTexture::get(bool border) { return border ? m_border_texture : m_center_texture; }

sf::Vector2<float> MapTexture::get_position() { return sf::Vector2<float>(static_cast<float>(global_offset.x), static_cast<float>(global_offset.y)); }

sf::Vector2<float> MapTexture::get_dimensions() const { return map_dimensions; }

} // namespace fornani::gui
