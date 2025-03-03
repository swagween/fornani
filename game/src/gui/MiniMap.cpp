
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
}

void MiniMap::bake(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int room, bool current, bool undiscovered) {
	atlas.push_back(std::make_unique<MapTexture>(svc));
	m_texture_scale = atlas.back()->get_scale();
	if (current) { atlas.back()->set_current(); }
	atlas.back()->bake(svc, map, room, 1.f, current, undiscovered);
	extent.position.x = std::min(atlas.back()->get_position().x, extent.position.x);
	extent.size.x = std::max(atlas.back()->get_position().x + atlas.back()->get_dimensions().x, extent.size.x);
	extent.position.y = std::min(atlas.back()->get_position().y, extent.position.y);
	extent.size.y = std::max(atlas.back()->get_position().y + atlas.back()->get_dimensions().y, extent.size.y);

	// populate entity data for icons
	if (!map.is_minimap()) { return; } // don't care about test maps
	auto room_pos{atlas.back()->get_position()};
	if (map.save_point.id > 0) { m_markers.push_back({MapIconFlags::save, map.save_point.position * m_texture_scale + room_pos, room}); }
	for (auto& bed : map.beds) { m_markers.push_back({MapIconFlags::bed, bed.bounding_box.get_position() * m_texture_scale / svc.constants.cell_size + room_pos, room}); }
	for (auto& door : map.portals) {
		if (!door.activate_on_contact()) { m_markers.push_back({MapIconFlags::door, door.position * m_texture_scale / svc.constants.cell_size + room_pos, room}); }
	}
	if (current) { m_markers.push_back({MapIconFlags::nani, player.collider.get_center() * m_texture_scale / svc.constants.cell_size + room_pos, room}); }
	map.clear();
}

void MiniMap::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	border.setSize(svc.constants.f_screen_dimensions);
	ratio = 32.f / scale;
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam, sf::Sprite& icon_sprite) {
	static auto flash_frame{util::Circuit{2}};
	if (svc.ticker.every_x_frames(10)) { flash_frame.modulate(1); }
	view = svc.window->get_view();
	auto port = svc.window->get_viewport();
	port.size.x = m_port_dimensions.x / view.getSize().x;
	port.size.y = m_port_dimensions.y / view.getSize().y;

	port.position.x = m_port_position.x / view.getSize().x - cam.x / view.getSize().x;
	port.position.y = m_port_position.y / view.getSize().y - cam.y / view.getSize().y;

	view.setViewport(port);
	win.setView(view);

	if (port.size.x == 0.f || port.size.y == 0.f) { return; }

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
	}
	icon_sprite.setScale(svc.constants.texture_scale.componentWiseDiv(port.size));
	auto icon_lookup{136};
	auto icon_dim{6};
	for (auto& element : m_markers) {
		icon_sprite.setTextureRect(sf::IntRect{{icon_lookup + icon_dim * flash_frame.get(), static_cast<int>(element.type) * icon_dim}, {icon_dim, icon_dim}});
		icon_sprite.setPosition((element.position * ratio + position).componentWiseDiv(port.size));
		win.draw(icon_sprite);
	}
	m_cursor.setScale(svc.constants.texture_scale.componentWiseDiv(port.size));
	m_cursor.setPosition(svc.constants.f_center_screen);
	win.draw(m_cursor);
	svc.window->restore_view();
}

void MiniMap::clear_atlas() { atlas.clear(); }

void MiniMap::move(sf::Vector2<float> direction) {
	position -= direction * speed;
	position.x = std::clamp(position.x, -(extent.size.x) * ratio + view.getCenter().x, -(extent.position.x) * ratio + view.getCenter().x);
	position.y = std::clamp(position.y, -(extent.size.y) * ratio + view.getCenter().y, -(extent.position.y) * ratio + view.getCenter().y);
	previous_position = position;
}

void gui::MiniMap::zoom(float amount) {
	auto prev_ratio = ratio;
	scale = std::clamp(scale + amount, m_texture_scale, m_texture_scale * 16.f);
	ratio = 32.f / scale;
	auto r_delta = ratio - prev_ratio;
	auto sz{m_port_dimensions.componentWiseDiv(view.getSize())};
	center_position = (position - view.getCenter().componentWiseMul(sz)) / prev_ratio;
	if (std::abs(r_delta) > 0.f) { position += center_position * r_delta; }
}

void MiniMap::center() {
	for (auto& room : atlas) {
		if (room->is_current()) {
			auto sz{m_port_dimensions.componentWiseDiv(view.getSize())};
			position = (player_position + room->get_position()) * ratio - view.getCenter().componentWiseMul(sz);
			return;
		}
	}
}

void gui::MiniMap::set_port_position(sf::Vector2f to_position) { m_port_position = to_position; }

void gui::MiniMap::set_port_dimensions(sf::Vector2f to_dimensions) { m_port_dimensions = to_dimensions; }

} // namespace fornani::gui
