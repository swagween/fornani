#include "fornani/gui/MiniMap.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"

namespace gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : texture(svc), map_sprite{svc.assets.t_null} {
	background_color = svc.styles.colors.ui_black;
	background_color.a = 210;
	background.setFillColor(background_color);
	border.setOutlineColor(svc.styles.colors.ui_white);
	border.setOutlineThickness(-4.f);
	border.setFillColor(sf::Color::Transparent);
	room_border.setOutlineColor(svc.styles.colors.blue);
	room_border.setOutlineThickness(-2.f);
	room_border.setFillColor(sf::Color::Transparent);
	player_box.setFillColor(svc.styles.colors.periwinkle);
	player_box.setOutlineColor(svc.styles.colors.ui_white);
	player_box.setOutlineThickness(2.f);
	player_box.setSize({16.f, 16.f});
	player_box.setOrigin({8.f, 8.f});
	cursor.vert.setFillColor(svc.styles.colors.ui_white);
	cursor.vert.setSize({2.f, 16.f});
	cursor.vert.setOrigin({1.f, 8.f});
	cursor.horiz.setFillColor(svc.styles.colors.ui_white);
	cursor.horiz.setSize({16.f, 2.f});
	cursor.horiz.setOrigin({8.f, 1.f});
	toggle_scale();
}

void MiniMap::bake(automa::ServiceProvider& svc, world::Map& map, int room, bool current, bool undiscovered) {
	atlas.push_back(std::make_unique<MapTexture>(svc));
	if (current) { atlas.back()->set_current(); }
	atlas.back()->bake(svc, map, room, scale, current, undiscovered);
	extent.position.x = std::min(atlas.back()->get_position().x, extent.position.x);
	extent.size.x = std::max(atlas.back()->get_position().x + atlas.back()->get_dimensions().x, extent.size.x);
	extent.position.y = std::min(atlas.back()->get_position().y, extent.position.y);
	extent.size.y = std::max(atlas.back()->get_position().y + atlas.back()->get_dimensions().y, extent.size.y);
}

void MiniMap::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	view = svc.window->get_view();
	auto port = svc.window->get_viewport();
	port.size.x *= window_scale;
	port.size.y *= window_scale;
	port.position.x = (1.f - port.size.x) * 0.5f;
	port.position.y = (1.f - port.size.y) * 0.5f;
	view.setViewport(port);
	background.setSize(svc.constants.f_screen_dimensions);
	border.setSize(svc.constants.f_screen_dimensions);
	ratio = 32.f / scale;
	player_position = player.collider.physics.position;
	center_position = (position - view.getCenter()) / ratio;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	// render minimap
	global_ratio = ratio * 0.25f;
	win.setView(view);
	win.draw(background);
	if (svc.ticker.every_x_frames(10)) {
		room_border.getFillColor() == svc.styles.colors.ui_white ? room_border.setOutlineColor(svc.styles.colors.periwinkle) : room_border.setOutlineColor(svc.styles.colors.ui_white);
		player_box.getFillColor() == svc.styles.colors.periwinkle ? player_box.setFillColor(svc.styles.colors.ui_white) : player_box.setFillColor(svc.styles.colors.periwinkle);
	}
	for (auto& room : atlas) {
		if (room->is_current()) { player_box.setPosition((player_position / scale) + room->get_position() * ratio + position); }
		map_sprite.setTexture(room->get().getTexture());
		map_sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		map_sprite.setScale({global_ratio, global_ratio});
		map_sprite.setPosition(room->get_position() * ratio + position);
		room_border.setPosition(map_sprite.getPosition());
		room_border.setOrigin(map_sprite.getOrigin());
		room_border.setSize(map_sprite.getLocalBounds().size);
		room_border.setScale(map_sprite.getScale());
		room_border.setOutlineThickness(scale / 4.f);
		win.draw(map_sprite);
		auto tl = room->get_position() * ratio + position;
		auto br = tl + map_sprite.getLocalBounds().size * global_ratio;
		auto pos = view.getCenter();
		if( pos.x > tl.x && pos.x < br.x && pos.y > tl.y && pos.y < br.y) { win.draw(room_border); }
		win.draw(player_box);
	}
	cursor.vert.setPosition(svc.constants.f_center_screen);
	cursor.horiz.setPosition(svc.constants.f_center_screen);
	win.draw(cursor.vert);
	win.draw(cursor.horiz);
	win.draw(border);
	svc.window->restore_view();
}

void MiniMap::toggle_scale() {
	scalar.modulate(1);
	scale = std::pow(2.f, static_cast<float>(scalar.get()) + 2.f);
	ratio = 32.f / scale;
	texture.tile_box.setSize({ratio, ratio});
	texture.plat_box.setSize({ratio, ratio});
	texture.portal_box.setSize({ratio, ratio});
	texture.save_box.setSize({ratio, ratio});
	texture.breakable_box.setSize({ratio, ratio});
	if (scale == 4.f) {
		position += center_position * 6.f;
	} else {
		position -= center_position * ratio;
	}
}

void MiniMap::move(sf::Vector2<float> direction) {
	position -= direction * speed;
	position.x = std::clamp(position.x, -(extent.size.x) * ratio + view.getCenter().x, -(extent.position.x) * ratio + view.getCenter().x);
	position.y = std::clamp(position.y, -(extent.size.y) * ratio + view.getCenter().y, -(extent.position.y) * ratio + view.getCenter().y);
	previous_position = position;
}

void MiniMap::center() {
	for (auto& room : atlas) {
		if (room->is_current()) {
			position = -room->get_position() * ratio + view.getCenter() - (player_position / scale);
			return;
		}
	}
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
