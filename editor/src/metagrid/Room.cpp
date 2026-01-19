
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
	auto real_dimensions = sf::Vector2u{in.metadata["meta"]["dimensions"][0].as<unsigned int>(), in.metadata["meta"]["dimensions"][1].as<unsigned int>()};
	m_box.setFillColor(room_color_v);
	m_box.setSize(sf::Vector2f{dimensions} * spacing_v);
	m_texture.clear();
	if (!m_texture.resize(real_dimensions) || dimensions.x * dimensions.y == 0.f) { return; }
	auto cell = sf::RectangleShape{};
	cell.setFillColor(room_color_v);
	cell.setSize({1.f, 1.f});
	for (auto [i, tile] : std::views::enumerate(in.metadata["tile"]["layers"][in.metadata["tile"]["middleground"].as<int>()].as_array())) {
		if (tile.as<int>() > 0) {
			auto x = static_cast<float>(i % real_dimensions.x);
			auto y = static_cast<float>(i / real_dimensions.x);
			cell.setPosition({x, y});
			m_texture.draw(cell);
		}
	}
	m_texture.display();
}

bool Room::serialize(fornani::automa::ServiceProvider& svc) {
	m_data->metadata["meta"]["minimap"] = m_include_in_minimap;
	m_data->metadata["meta"]["metagrid"][0] = m_position.x;
	m_data->metadata["meta"]["metagrid"][1] = m_position.y;
	auto msg = std::string{};
	for (auto const& entry : m_data->metadata["entities"]["inspectables"].as_array()) {
		msg = entry["series"][0]["suite"][0][0]["message"].as_string();
		NANI_LOG_INFO(m_logger, "Message before write: {}", msg);
	}
	return m_data->metadata.to_file((svc.finder.paths.levels / std::filesystem::path{m_data->region_label} / std::filesystem::path{m_data->room_label + ".json"}).string());
}

void Room::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto& color = m_include_in_minimap ? room_color_v : excluded_room_color_v;
	auto& h_color = m_include_in_minimap ? highighted_room_color_v : highlighted_excluded_room_color_v;
	m_highlighted ? m_box.setFillColor(h_color) : m_box.setFillColor(sf::Color::Transparent);
	m_highlighted ? m_box.setOutlineColor(fornani::colors::pioneer_red) : m_box.setOutlineColor(sf::Color{79, 22, 32});
	if (no_border) { m_box.setOutlineColor(sf::Color::Transparent); }
	m_highlighted ? m_box.setOutlineThickness(-2.f) : m_box.setOutlineThickness(-1.f);
	m_box.setPosition(get_board_position() + cam);
	auto sprite = sf::Sprite{m_texture.getTexture()};
	m_include_in_minimap ? sprite.setColor(sf::Color::White) : sprite.setColor(fornani::colors::periwinkle);
	sprite.setPosition(m_box.getPosition());
	sprite.scale({spacing_v / fornani::constants::f_chunk_size, spacing_v / fornani::constants::f_chunk_size});
	win.draw(sprite);
	win.draw(m_box);
}

} // namespace pi
