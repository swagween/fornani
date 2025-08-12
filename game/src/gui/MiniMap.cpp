
#include "fornani/gui/MiniMap.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::gui {

MiniMap::MiniMap(automa::ServiceProvider& svc) : m_texture(svc), m_map_sprite{svc.assets.get_texture("null")}, m_cursor{svc.assets.get_texture("map_cursor")}, m_speed{64.f} {
	m_border.setOutlineColor(colors::pioneer_dark_red);
	m_border.setOutlineThickness(-4.f);
	m_border.setFillColor(sf::Color::Transparent);
	m_border.setSize(svc.window->f_screen_dimensions());
	m_cursor.setScale(constants::f_scale_vec);
	m_cursor.setOrigin({7.f, 7.f});
}

void MiniMap::bake(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int room, bool current, bool undiscovered) {
	m_atlas.push_back(std::make_unique<MapTexture>(svc));
	auto& current_map{m_atlas.back()};
	m_texture_scale = current_map->get_scale();
	if (current) { current_map->set_current(); }
	current_map->bake(svc, map, room, 1.f, current, undiscovered);
	m_extent.position.x = std::min(current_map->get_position().x, m_extent.position.x);
	m_extent.size.x = std::max(current_map->get_position().x + current_map->get_dimensions().x, m_extent.size.x);
	m_extent.position.y = std::min(current_map->get_position().y, m_extent.position.y);
	m_extent.size.y = std::max(current_map->get_position().y + current_map->get_dimensions().y, m_extent.size.y);

	// populate entity data for icons
	if (!map.is_minimap()) { return; } // don't care about test maps
	auto room_pos{current_map->get_position()};
	if (map.save_point) { m_markers.push_back({MapIconFlags::save, map.save_point->get_world_position() * m_texture_scale / constants::f_cell_size + room_pos, room}); }
	for (auto& bed : map.beds) { m_markers.push_back({MapIconFlags::bed, bed.bounding_box.get_position() * m_texture_scale / constants::f_cell_size + room_pos, room}); }
	for (auto& door : map.portals) {
		if (!door.activate_on_contact()) { m_markers.push_back({MapIconFlags::door, door.get_world_position() * m_texture_scale / constants::f_cell_size + room_pos, room}); }
	}
	if (current) {
		m_player_position = player.collider.get_center() * m_texture_scale / constants::f_cell_size + room_pos;
		m_markers.push_back({MapIconFlags::nani, m_player_position, room});
	}
	map.clear();
}

void MiniMap::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2f cam, sf::Sprite& icon_sprite) {
	static auto flash_frame{util::Circuit{2}};
	if (svc.ticker.every_x_frames(10)) { flash_frame.modulate(1); }
	m_view = svc.window->get_view();
	auto port = svc.window->get_viewport();
	port.size.x = m_port_dimensions.x / m_view.getSize().x;
	port.size.y = m_port_dimensions.y / m_view.getSize().y;

	port.position.x = m_port_position.x / m_view.getSize().x - cam.x / m_view.getSize().x;
	port.position.y = m_port_position.y / m_view.getSize().y - cam.y / m_view.getSize().y;

	m_view.setScissor(svc.window->get_viewport());

	m_view.setViewport(port);
	win.setView(m_view);

	if (port.size.x == 0.f || port.size.y == 0.f) { return; }

	for (auto& room : m_atlas) {
		if (room->to_ignore()) { continue; }
		m_map_sprite.setTexture(room->get().getTexture());
		m_map_sprite.setTextureRect(sf::IntRect({}, static_cast<sf::Vector2<int>>(room->get().getSize())));
		m_map_sprite.setScale(get_ratio_vec2().componentWiseDiv(port.size));
		m_map_sprite.setPosition((room->get_position() * get_ratio() + m_physics.position).componentWiseDiv(port.size));
		auto outline{sf::Sprite{room->get(true).getTexture()}};
		outline.setScale(get_ratio_vec2().componentWiseDiv(port.size));
		for (auto i{-1}; i < 2; ++i) {
			for (auto j{-1}; j < 2; ++j) {
				if ((std::abs(i) % 2 == 0 && std::abs(j) % 2 == 0) || (std::abs(i) % 2 == 1 && std::abs(j) % 2 == 1)) { continue; }
				auto skew{sf::Vector2f{static_cast<float>(i), static_cast<float>(j)}};
				outline.setPosition((room->get_position() * get_ratio() + m_physics.position).componentWiseDiv(port.size) + (skew * 2.f).componentWiseDiv(port.size));
				win.draw(outline);
			}
		}
		win.draw(m_map_sprite);
	}
	icon_sprite.setScale(constants::f_scale_vec.componentWiseDiv(port.size));
	auto icon_lookup{136};
	auto icon_dim{6};
	for (auto& element : m_markers) {
		icon_sprite.setTextureRect(sf::IntRect{{icon_lookup + icon_dim * flash_frame.get(), static_cast<int>(element.type) * icon_dim}, {icon_dim, icon_dim}});
		icon_sprite.setPosition((element.position * get_ratio() + m_physics.position).componentWiseDiv(port.size));
		if (element.type == MapIconFlags::nani) { icon_sprite.setScale(icon_sprite.getScale().componentWiseMul(player.get_facing_scale())); }
		win.draw(icon_sprite);
	}
	m_cursor.setScale(constants::f_scale_vec.componentWiseDiv(port.size));
	m_cursor.setPosition(svc.window->f_center_screen());
	win.draw(m_cursor);
	svc.window->restore_view();
}

void MiniMap::update() {
	m_physics.simple_update();
	auto bounds{sf::FloatRect{{-(m_extent.size.x) * get_ratio() + m_view.getCenter().x, -(m_extent.size.y) * get_ratio() + m_view.getCenter().y},
							  {-(m_extent.position.x) * get_ratio() + m_view.getCenter().x, -(m_extent.position.y) * get_ratio() + m_view.getCenter().y}}};
	m_physics.position.x = ccm::ext::clamp(m_physics.position.x, bounds.position.x, bounds.size.x);
	m_physics.position.y = ccm::ext::clamp(m_physics.position.y, bounds.position.y, bounds.size.y);
	m_pan_limit_x = m_physics.position.x == bounds.position.x || m_physics.position.x == bounds.size.x;
	m_pan_limit_y = m_physics.position.y == bounds.position.y || m_physics.position.y == bounds.size.y;
}

void MiniMap::clear_atlas() { m_atlas.clear(); }

void MiniMap::move(sf::Vector2f direction) {
	auto speed = m_speed;
	if (ccm::abs(direction.x) + ccm::abs(direction.y) > 1.f) { speed /= ccm::sqrt(2.f); }
	m_steering.target(m_physics, m_physics.position - direction * speed, 0.002f);
}

void MiniMap::zoom(float amount) {
	auto prev_ratio = get_ratio();
	auto max_scale{64.f};
	m_scale = ccm::ext::clamp(m_scale + amount, m_texture_scale, m_texture_scale * max_scale);
	m_zoom_limit = m_scale == m_texture_scale || m_scale == m_texture_scale * max_scale;
	auto r_delta = get_ratio() - prev_ratio;
	auto sz{m_port_dimensions.componentWiseDiv(m_view.getSize())};
	m_center_position = (m_physics.position - m_view.getCenter().componentWiseMul(sz)) / prev_ratio;
	if (ccm::abs(r_delta) > 0.f) { m_physics.position += m_center_position * r_delta; }
}

void MiniMap::center() {
	auto sz{m_port_dimensions.componentWiseDiv(m_view.getSize())};
	m_physics.position = -m_player_position * get_ratio() + m_view.getCenter().componentWiseMul(sz);
}

void MiniMap::set_port_position(sf::Vector2f to_position) { m_port_position = to_position; }

void MiniMap::set_port_dimensions(sf::Vector2f to_dimensions) { m_port_dimensions = to_dimensions; }

} // namespace fornani::gui
