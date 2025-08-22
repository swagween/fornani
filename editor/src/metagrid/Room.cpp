
#include <editor/metagrid/Room.hpp>
#include <fornani/utils/Constants.hpp>

namespace pi {

Room::Room(dj::Json const& in) {
	m_position = sf::Vector2i{in["meta"]["metagrid"][0].as<int>(), in["meta"]["metagrid"][1].as<int>()};
	m_include_in_minimap = in["meta"]["minimap"].as_bool();
	auto dimensions = sf::Vector2u{in["meta"]["dimensions"][0].as<unsigned int>(), in["meta"]["dimensions"][1].as<unsigned int>()} / fornani::constants::u32_chunk_size;
	m_box.setFillColor(room_color_v);
	m_box.setSize(sf::Vector2f{dimensions} * spacing_v);
}

void Room::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto& color = m_include_in_minimap ? room_color_v : excluded_room_color_v;
	auto& h_color = m_include_in_minimap ? highighted_room_color_v : highlighted_excluded_room_color_v;
	m_highlighted ? m_box.setFillColor(h_color) : m_box.setFillColor(color);
	m_highlighted ? m_box.setOutlineColor(fornani::colors::ui_white) : m_box.setOutlineColor(fornani::colors::dark_grey);
	m_highlighted ? m_box.setOutlineThickness(1.f) : m_box.setOutlineThickness(-1.f);
	m_box.setPosition(get_board_position() + cam);
	win.draw(m_box);
}

} // namespace pi
