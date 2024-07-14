#include "MapTexture.hpp"
#include "../service/ServiceProvider.hpp"
#include "../level/Map.hpp"
#include <algorithm>

namespace gui {

MapTexture::MapTexture(automa::ServiceProvider& svc) {
	tile_color = svc.styles.colors.blue;
	tile_color.a = 235;
	tile_box.setFillColor(tile_color);
	tile_box.setSize({4.f, 4.f});
	plat_box.setFillColor(svc.styles.colors.periwinkle);
	plat_box.setSize({4.f, 4.f});
	portal_box.setFillColor(svc.styles.colors.green);
	portal_box.setSize({4.f, 4.f});
	breakable_box.setFillColor(svc.styles.colors.dark_goldenrod);
	breakable_box.setSize({4.f, 4.f});
	save_box.setFillColor(svc.styles.colors.periwinkle);
	save_box.setSize({4.f, 4.f});
}

void MapTexture::bake(automa::ServiceProvider& svc, world::Map& map, int room, float scale, bool current, bool undiscovered) {
	map.load(svc, room, true);
	global_offset = map.metagrid_coordinates * 16;
	auto const& middleground = map.get_layers().at(world::MIDDLEGROUND);
	map_texture.create(map.dimensions.x * static_cast<unsigned int>((32.f / scale)), map.dimensions.y * static_cast<unsigned int>(32.f / scale));
	map_texture.clear(sf::Color::Transparent);
	for (auto& cell : middleground.grid.cells) {
		if (cell.is_occupied() && !cell.is_breakable()) {
			tile_box.setPosition(cell.position / scale);
			sf::Color diff = tile_color;
			auto darkener = current ? 0 : 40;
			auto g_diff = cell.value / 8;
			diff.g = std::clamp(diff.g + g_diff - darkener, 0, 255);
			auto r_diff = cell.value / 4;
			diff.r = std::clamp(diff.r + r_diff - darkener / 2, 0, 255);
			auto b_diff = cell.value / 4;
			diff.b = std::clamp(diff.b + b_diff - darkener / 2, 0, 255);
			if (cell.surrounded) { diff.b = std::clamp(diff.b - 40, 0, 255); }
			if (cell.surrounded) { diff.g = std::clamp(diff.g - 60, 0, 255); }
			if (cell.surrounded) { diff.r = std::clamp(diff.r - 20, 0, 255); }
			tile_box.setFillColor(current ? diff : diff);
			map_texture.draw(tile_box);
		}
	}
	for (auto& portal : map.portals) {
		portal_box.setPosition(portal.position / scale);
		portal_box.setSize(portal.dimensions / scale);
		map_texture.draw(portal_box);
	}
	for (auto& breakable : map.breakables) {
		breakable_box.setPosition(breakable.get_bounding_box().position / scale);
		breakable_box.setSize(breakable.get_bounding_box().dimensions / scale);
		map_texture.draw(breakable_box);
	}
	if (map.save_point.id > 0) {
		save_box.setPosition(sf::Vector2 <float>(map.save_point.scaled_position) * 32.f / scale);
		map_texture.draw(save_box);
	}
	if (undiscovered) { 
		curtain.setSize({static_cast<float>(map.dimensions.x) * (32.f / scale), static_cast<float>(map.dimensions.y) * (32.f / scale)});
		curtain.setFillColor(svc.styles.colors.navy_blue);
		map_texture.draw(curtain);
	}

	map_texture.display();
}

sf::Sprite MapTexture::sprite() { return sf::Sprite(map_texture.getTexture()); }

sf::RenderTexture& MapTexture::get() { return map_texture; }

sf::Vector2<float> MapTexture::get_position() { return sf::Vector2<float>(static_cast<float>(global_offset.x), static_cast<float>(global_offset.y)); }

} // namespace text
