
#include <editor/metagrid/Room.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/DataManager.hpp>
#include <fornani/utils/Constants.hpp>

namespace pi {

Room::Room(fornani::automa::ServiceProvider& svc, fornani::data::MapData& in) : id{in.metadata["meta"]["room_id"].as<int>()}, m_label{svc.text.fonts.config}, m_biome{svc.text.fonts.config}, m_data(&in) {
	m_label.setString(in.room_label);
	m_biome.setString(in.metadata["meta"]["biome"].as_string());
	m_position = sf::Vector2i{in.metadata["meta"]["metagrid"][0].as<int>(), in.metadata["meta"]["metagrid"][1].as<int>()};
	m_include_in_minimap = in.metadata["meta"]["minimap"].as_bool();
	auto dimensions = sf::Vector2u{in.metadata["meta"]["dimensions"][0].as<unsigned int>(), in.metadata["meta"]["dimensions"][1].as<unsigned int>()} / fornani::constants::u32_chunk_size;
	m_box.setFillColor(room_color_v);
	m_box.setSize(sf::Vector2f{dimensions} * spacing_v);
}

bool Room::serialize(fornani::automa::ServiceProvider& svc) {
	m_data->metadata["meta"]["minimap"] = m_include_in_minimap;
	m_data->metadata["meta"]["metagrid"][0] = m_position.x;
	m_data->metadata["meta"]["metagrid"][1] = m_position.y;
	return m_data->metadata.to_file((svc.finder.paths.levels / std::filesystem::path{m_data->biome_label} / std::filesystem::path{m_data->room_label + ".json"}).string().c_str());
}

void Room::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto& color = m_include_in_minimap ? room_color_v : excluded_room_color_v;
	auto& h_color = m_include_in_minimap ? highighted_room_color_v : highlighted_excluded_room_color_v;
	m_highlighted ? m_box.setFillColor(h_color) : m_box.setFillColor(color);
	m_highlighted ? m_box.setOutlineColor(fornani::colors::pioneer_red) : m_box.setOutlineColor(sf::Color{79, 22, 32});
	m_highlighted ? m_box.setOutlineThickness(-2.f) : m_box.setOutlineThickness(-1.f);
	m_box.setPosition(get_board_position() + cam);
	win.draw(m_box);
}

} // namespace pi
