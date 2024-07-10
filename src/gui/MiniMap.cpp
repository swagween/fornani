#include "MiniMap.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../level/Map.hpp"

namespace gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : texture(svc) {
	background_color = svc.styles.colors.ui_black;
	background_color.a = 120;
	background.setFillColor(background_color);
	border.setOutlineColor(svc.styles.colors.ui_white);
	border.setOutlineThickness(-4.f);
	border.setFillColor(sf::Color::Transparent);
	player_box.setFillColor(svc.styles.colors.periwinkle);
	player_box.setOutlineColor(svc.styles.colors.ui_white);
	player_box.setOutlineThickness(2.f);
	player_box.setSize({16.f, 16.f});
	player_box.setOrigin({8.f, 8.f});
	toggle_scale();
}

void MiniMap::bake(automa::ServiceProvider& svc, world::Map& map, int room, bool current) {
	atlas.push_back(std::make_unique<MapTexture>(svc));
	if (current) { atlas.back()->set_current(); }
	atlas.back()->bake(svc, map, room, scale, current);
}

void MiniMap::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	view = sf::View(sf::FloatRect(0.0f, 0.0f, svc.constants.screen_dimensions.x, svc.constants.screen_dimensions.y));
	view.setViewport(sf::FloatRect(0.2f, 0.2f, 0.6f, 0.6f));
	background.setSize(svc.constants.f_screen_dimensions);
	border.setSize(svc.constants.f_screen_dimensions);
	speed = 10.f / scale;
	ratio = 32.f / scale;
	player_position = player.collider.physics.position;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	// render minimap
	global_ratio = ratio * 0.25f;
	win.setView(view);
	win.draw(background);
	if (svc.ticker.every_x_frames(10)) { player_box.getFillColor() == svc.styles.colors.periwinkle ? player_box.setFillColor(svc.styles.colors.ui_white) : player_box.setFillColor(svc.styles.colors.periwinkle); }
	for (auto& room : atlas) {
		if (room->is_current()) { player_box.setPosition((player_position / scale) + room->get_position() * ratio + position); }
		map_sprite.setTexture(room->get().getTexture());
		map_sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		map_sprite.setScale({global_ratio, global_ratio});
		map_sprite.setPosition(room->get_position() * ratio + position);
		win.draw(map_sprite);
		win.draw(player_box);
	}
	win.draw(border);
	win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y}));
}

void MiniMap::toggle_scale() {
	//std::cout << "xpos: " << position.x << "\nypos: " << position.y << "\n";
	scalar.modulate(1);
	scale = std::pow(2, scalar.get() + 2);
	speed = 10.f / scale;
	ratio = 32.f / scale;
	texture.tile_box.setSize({ratio, ratio});
	texture.plat_box.setSize({ratio, ratio});
	texture.portal_box.setSize({ratio, ratio});
	texture.save_box.setSize({ratio, ratio});
	texture.breakable_box.setSize({ratio, ratio});
	//as scale goes up, position must increase
	// -221 ->> 139

	// -951, -224 (4)
	// -221, 14 (8)
	// 134, 153 (16)
	// diffs: 720, 360
	if (scale == 4.f) {
		position = previous_position;
		previous_position = position;
	} else {
		position.x += 90 * scale;
		position.y += 90 * scale;
	}
}

void MiniMap::move(sf::Vector2<float> direction) {
	position -= direction * speed;
	previous_position = position;
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
