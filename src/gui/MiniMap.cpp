#include "MiniMap.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../level/Map.hpp"

namespace gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) {
	background_color = svc.styles.colors.ui_black;
	background_color.a = 120;
	tile_color = svc.styles.colors.blue;
	tile_color.a = 225;
	background.setFillColor(background_color);
	background.setSize(static_cast<sf::Vector2<float>>(svc.constants.screen_dimensions) - sf::Vector2<float>{140.f, 140.f});
	background.setOrigin(background.getSize() * 0.5f);
	tile_box.setFillColor(svc.styles.colors.blue);
	tile_box.setSize({4.f, 4.f});
	player_box.setFillColor(svc.styles.colors.bright_orange);
	player_box.setSize({4.f, 4.f});
	plat_box.setFillColor(svc.styles.colors.periwinkle);
	plat_box.setSize({4.f, 4.f});
}

void MiniMap::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	view = sf::View(sf::FloatRect(0.0f, 0.0f, svc.constants.screen_dimensions.x, svc.constants.screen_dimensions.y));
	view.setViewport(sf::FloatRect(0.2f, 0.2f, 0.8f, 0.8f));
	background.setPosition(svc.constants.f_center_screen);

	auto const& middleground = map.layers.at(world::MIDDLEGROUND);
	map_texture.clear(sf::Color::Transparent);
	map_texture.create(middleground.grid.dimensions.x * 4.f, middleground.grid.dimensions.y * 4.f);
	for (auto& cell : middleground.grid.cells) {
		if (cell.is_occupied() && !cell.is_breakable()) {
			tile_box.setPosition(cell.position / scale);
			sf::Color diff = tile_color;
			auto g_diff = cell.value / 3;
			diff.g = std::clamp(diff.g + g_diff, 0, 255);
			auto r_diff = cell.value / 4;
			diff.r = std::clamp(diff.r + r_diff, 0, 255);
			auto b_diff = cell.value / 8;
			diff.b = std::clamp(diff.b + b_diff, 0, 255);
			if (cell.surrounded) { diff.b = std::clamp(diff.b - 40, 0, 255); }
			if (cell.surrounded) { diff.g = std::clamp(diff.g - 60, 0, 255); }
			if (cell.surrounded) { diff.r = std::clamp(diff.r - 20, 0, 255); }
			tile_box.setFillColor(diff);
			map_texture.draw(tile_box);
		}
	}
	for (auto& plat : map.platforms) {
		plat_box.setPosition(plat.physics.position / scale);
		plat_box.setSize(plat.dimensions / scale);
		map_texture.draw(plat_box);
	}
	player_box.setPosition(player.collider.physics.position / scale);
	if (svc.ticker.every_x_ticks(40)) { player_box.getFillColor() == svc.styles.colors.dark_orange ? player_box.setFillColor(svc.styles.colors.bright_orange) : player_box.setFillColor(svc.styles.colors.dark_orange); }
	map_texture.draw(player_box);
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	// render minimap
	//win.setView(view);
	//win.draw(background);
	map_texture.display();
	map_sprite.setTexture(map_texture.getTexture());
	map_sprite.setOrigin(map_sprite.getLocalBounds().getSize() * 0.5f);
	map_sprite.setPosition(svc.constants.f_center_screen);
	//map_sprite.setPosition(-cam);
	win.draw(map_sprite);
	//win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y}));
}

void MiniMap::toggle_scale() {
	scale = (scale == 16.f) ? 8.f : 16.f;
	tile_box.setSize({32.f / scale, 32.f / scale});
	plat_box.setSize({32.f / scale, 32.f / scale});
	player_box.setSize({32.f / scale, 32.f / scale});
}

void Chunk::generate() {
	switch (type) {
	case ChunkType::top_left: break;
	case ChunkType::top: break;
	case ChunkType::top_right: break;
	case ChunkType::bottom_left: break;
	case ChunkType::bottom: break;
	case ChunkType::bottom_right: break;
	case ChunkType::left: break;
	case ChunkType::right: break;
	case ChunkType::inner: break;
	}
}

} // namespace gui
