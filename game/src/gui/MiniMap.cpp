
#include "fornani/gui/MiniMap.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : texture(svc), map_sprite{svc.assets.t_null}, m_cursor{svc.assets.get_texture("map_cursor")} {
	border.setOutlineColor(svc.styles.colors.pioneer_dark_red);
	border.setOutlineThickness(-4.f);
	border.setFillColor(sf::Color::Transparent);
	player_box.setFillColor(svc.styles.colors.pioneer_red);
	player_box.setSize({16.f, 16.f});
	player_box.setOrigin({8.f, 8.f});
	m_cursor.setScale(svc.constants.texture_scale);
	m_cursor.setOrigin({7.f, 7.f});
	toggle_scale();
}

void MiniMap::bake(automa::ServiceProvider& svc, world::Map& map, int room, bool current, bool undiscovered) {
	atlas.push_back(std::make_unique<MapTexture>(svc));
	if (current) { atlas.back()->set_current(); }
	atlas.back()->bake(svc, map, room, 1.f, current, undiscovered);
	extent.position.x = std::min(atlas.back()->get_position().x, extent.position.x);
	extent.size.x = std::max(atlas.back()->get_position().x + atlas.back()->get_dimensions().x, extent.size.x);
	extent.position.y = std::min(atlas.back()->get_position().y, extent.position.y);
	extent.size.y = std::max(atlas.back()->get_position().y + atlas.back()->get_dimensions().y, extent.size.y);
}

void MiniMap::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	border.setSize(svc.constants.f_screen_dimensions);
	ratio = 32.f / scale;
	player_position = player.collider.physics.position / svc.constants.cell_size;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	view = svc.window->get_view();
	auto port = svc.window->get_viewport();
	port.size.x = m_port_dimensions.x / view.getSize().x;
	port.size.y = m_port_dimensions.y / view.getSize().y;

	port.position.x = m_port_position.x / view.getSize().x - cam.x / view.getSize().x;
	port.position.y = m_port_position.y / view.getSize().y - cam.y / view.getSize().y;

	view.setViewport(port);
	win.setView(view);

	if (svc.ticker.every_x_frames(10)) { player_box.getFillColor() == svc.styles.colors.pioneer_red ? player_box.setFillColor(svc.styles.colors.ui_white) : player_box.setFillColor(svc.styles.colors.pioneer_red); }
	for (auto& room : atlas) {
		if (room->to_ignore()) { continue; }
		if (room->is_current()) { player_box.setPosition(((player_position + room->get_position()) * ratio + position).componentWiseDiv(port.size)); }
		map_sprite.setTexture(room->get().getTexture());
		map_sprite.setTextureRect(sf::IntRect({}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		map_sprite.setScale(sf::Vector2f{ratio, ratio}.componentWiseDiv(port.size));
		map_sprite.setPosition((room->get_position() * ratio + position).componentWiseDiv(port.size));
		auto outline{sf::Sprite{room->get(true).getTexture()}};
		outline.setScale(sf::Vector2f{ratio, ratio}.componentWiseDiv(port.size));
		for (auto i{-1}; i < 2; ++i) {
			for (auto j{-1}; j < 2; ++j) {
				if ((std::abs(i) % 2 == 0 && std::abs(j) % 2 == 0) || (std::abs(i) % 2 == 1 && std::abs(j) % 2 == 1)) { continue; }
				auto skew{sf::Vector2f{static_cast<float>(i), static_cast<float>(j)}};
				outline.setPosition((room->get_position() * ratio + position).componentWiseDiv(port.size) + (skew * 2.f).componentWiseDiv(port.size));
				win.draw(outline);
			}
		}
		win.draw(map_sprite);
		win.draw(player_box);
	}
	m_cursor.setScale(svc.constants.texture_scale.componentWiseDiv(port.size));
	m_cursor.setPosition(svc.constants.f_center_screen);
	win.draw(m_cursor);
	svc.window->restore_view();
}

void MiniMap::clear_atlas() { atlas.clear(); }

void MiniMap::toggle_scale() {
	auto prev = scale;
	scalar.modulate(1);
	scale = std::pow(2.f, static_cast<float>(scalar.get()) + 2.f);
	ratio = 32.f / scale;
	texture.tile_box.setSize({ratio, ratio});
	texture.plat_box.setSize({ratio, ratio});
	texture.portal_box.setSize({ratio, ratio});
	texture.save_box.setSize({ratio, ratio});
	texture.breakable_box.setSize({ratio, ratio});
}

void MiniMap::move(sf::Vector2<float> direction) {
	position -= direction * speed;
	position.x = std::clamp(position.x, -(extent.size.x) * ratio + view.getCenter().x, -(extent.position.x) * ratio + view.getCenter().x);
	position.y = std::clamp(position.y, -(extent.size.y) * ratio + view.getCenter().y, -(extent.position.y) * ratio + view.getCenter().y);
	previous_position = position;
}

void gui::MiniMap::zoom(float amount) {
	auto prev_ratio = ratio;
	scale = std::clamp(scale + amount, 1.f, 16.f);
	ratio = 32.f / scale;
	auto r_delta = ratio - prev_ratio;
	auto sz{m_port_dimensions.componentWiseDiv(view.getSize())};
	center_position = (position - view.getCenter().componentWiseMul(sz)) / ratio;
	if (std::abs(r_delta) > 0.f) { position += center_position * r_delta; }
}

void MiniMap::center() {
	for (auto& room : atlas) {
		if (room->is_current()) {
			position = -room->get_position() * scale + view.getCenter();
			return;
		}
	}
}

void gui::MiniMap::set_port_position(sf::Vector2f to_position) { m_port_position = to_position; }

void gui::MiniMap::set_port_dimensions(sf::Vector2f to_dimensions) { m_port_dimensions = to_dimensions; }

} // namespace fornani::gui
