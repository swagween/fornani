
#include "fornani/gui/MiniMap.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : texture(svc), map_sprite{svc.assets.t_null}, window_scale{0.67f} {
	border.setOutlineColor(svc.styles.colors.pioneer_dark_red);
	border.setOutlineThickness(-4.f);
	border.setFillColor(sf::Color::Transparent);
	player_box.setFillColor(svc.styles.colors.pioneer_red);
	player_box.setSize({16.f, 16.f});
	player_box.setOrigin({8.f, 8.f});
	cursor.vert.setFillColor(svc.styles.colors.pioneer_red);
	cursor.vert.setSize({4.f, 16.f});
	cursor.vert.setOrigin({2.f, 8.f});
	cursor.horiz.setFillColor(svc.styles.colors.pioneer_red);
	cursor.horiz.setSize({16.f, 4.f});
	cursor.horiz.setOrigin({8.f, 2.f});
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
	border.setSize(svc.constants.f_screen_dimensions);
	ratio = 32.f / scale;
	player_position = player.collider.physics.position;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	view = svc.window->get_view();
	auto port = svc.window->get_viewport();
	port.size *= window_scale;

	// TODO: these mysterious values will be the RectPath position divided by screen dimensions
	port.position.x = 0.2396f - cam.x / view.getSize().x;
	port.position.y = 0.18f - cam.y / view.getSize().y;

	view.setViewport(port);
	center_position = ((position - view.getCenter()) / ratio);
	// render minimap
	global_ratio = ratio * 0.25f;
	win.setView(view);
	// win.draw(background);
	if (svc.ticker.every_x_frames(10)) { player_box.getFillColor() == svc.styles.colors.pioneer_red ? player_box.setFillColor(svc.styles.colors.ui_white) : player_box.setFillColor(svc.styles.colors.pioneer_red); }
	for (auto& room : atlas) {
		if (room->to_ignore()) { continue; }
		if (room->is_current()) { player_box.setPosition(((player_position / scale) / window_scale + room->get_position() * ratio + position)); }
		map_sprite.setTexture(room->get().getTexture());
		map_sprite.setTextureRect(sf::IntRect({0, 0}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		map_sprite.setScale(sf::Vector2f{global_ratio, global_ratio} / window_scale);
		map_sprite.setPosition(room->get_position() * ratio + position);
		auto outline{sf::Sprite{room->get(true).getTexture()}};
		outline.setScale(sf::Vector2f{global_ratio, global_ratio} / window_scale);
		for (auto i{-1}; i < 2; ++i) {
			for (auto j{-1}; j < 2; ++j) {
				if ((std::abs(i) % 2 == 0 && std::abs(j) % 2 == 0) || (std::abs(i) % 2 == 1 && std::abs(j) % 2 == 1)) { continue; }
				auto skew{sf::Vector2f{static_cast<float>(i), static_cast<float>(j)}};
				outline.setPosition((room->get_position() * ratio + position) + skew * 2.f / window_scale);
				win.draw(outline);
			}
		}
		win.draw(map_sprite);
		auto tl = room->get_position() * ratio + position;
		auto br = tl + map_sprite.getLocalBounds().size * global_ratio;
		auto pos = view.getCenter();
		win.draw(player_box);
	}
	cursor.vert.setPosition(svc.constants.f_center_screen);
	cursor.horiz.setPosition(svc.constants.f_center_screen);
	win.draw(cursor.vert);
	win.draw(cursor.horiz);
	win.draw(border);
	svc.window->restore_view();
}

void MiniMap::clear_atlas() { atlas.clear(); }

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
			position = -room->get_position() * ratio + view.getCenter() * window_scale;
			return;
		}
	}
}

} // namespace fornani::gui
