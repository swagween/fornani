#include "MiniMap.hpp"
#include "../service/ServiceProvider.hpp"
#include "../entities/player/Player.hpp"
#include "../level/Map.hpp"

namespace gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : texture(svc) {
	background_color = svc.styles.colors.ui_black;
	background_color.a = 120;
	player_box.setFillColor(svc.styles.colors.bright_orange);
	player_box.setSize({8.f, 8.f});
	player_box.setOrigin({4.f, 4.f});
	ratio = 32.f / scale;
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
	background.setPosition(svc.constants.f_center_screen);
	speed = 10.f / scale;
	ratio = 32.f / scale;
	player_position = player.collider.physics.position;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	// render minimap
	global_ratio = ratio * 0.25f;
	win.setView(view);

	for (auto& room : atlas) {
		if (room->is_current()) { player_box.setPosition((player_position / scale) + room->get_position() * ratio + position); }
		if (svc.ticker.every_x_frames(10)) { player_box.getFillColor() == svc.styles.colors.dark_orange ? player_box.setFillColor(svc.styles.colors.bright_orange) : player_box.setFillColor(svc.styles.colors.dark_orange); }
		map_sprite.setTexture(room->get().getTexture());
		map_sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		map_sprite.setScale({global_ratio, global_ratio});
		map_sprite.setPosition(room->get_position() * ratio + position);
		win.draw(map_sprite);
		win.draw(player_box);
	}
	win.setView(sf::View(sf::FloatRect{0.f, 0.f, (float)svc.constants.screen_dimensions.x, (float)svc.constants.screen_dimensions.y}));
}

void MiniMap::toggle_scale() {
	std::cout << "xpos: " << position.x << "\nypos: " << position.y << "\n";
	scalar.modulate(1);
	scale = std::pow(2, scalar.get() + 2);
	speed = 10.f / scale;
	ratio = 32.f / scale;
	texture.tile_box.setSize({ratio, ratio});
	texture.plat_box.setSize({ratio, ratio});
	texture.portal_box.setSize({ratio, ratio});
	texture.save_box.setSize({ratio, ratio});
	texture.breakable_box.setSize({ratio, ratio});
	player_box.setSize({ratio * 2.f, ratio * 2.f});
	position = position;
}

void MiniMap::move(sf::Vector2<float> direction) { position -= direction * speed; }

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
