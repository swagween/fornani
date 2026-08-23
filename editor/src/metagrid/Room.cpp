
#include <editor/metagrid/Room.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/DataManager.hpp>
#include <fornani/utils/Constants.hpp>

namespace pi {

Room::Room(fornani::automa::ServiceProvider& svc, fornani::data::MapData& in) : id{in.metadata["meta"]["room_id"].as<int>()}, m_label{svc.text.fonts.config.font}, m_biome{svc.text.fonts.config.font}, m_data(&in) {
	m_label.setString(in.room_label);
	m_biome.setString(in.metadata["meta"]["biome"].as_string());
	m_status = static_cast<RoomStatus>(in.metadata["meta"]["development_status"].as<int>());
	m_position = sf::Vector2i{in.metadata["meta"]["metagrid"][0].as<int>(), in.metadata["meta"]["metagrid"][1].as<int>()};
	set_flag(RoomFlags::include_in_minimap, in.metadata["meta"]["minimap"].as_bool());
	set_flag(RoomFlags::use_template, in.metadata["meta"]["use_template"].as_bool());
	set_flag(RoomFlags::interior, in.metadata["meta"]["properties"]["interior"].as_bool());
	set_flag(RoomFlags::day_night_shift, in.metadata["meta"]["properties"]["day_night_shift"].as_bool());
	auto dimensions = sf::Vector2u{in.metadata["meta"]["dimensions"][0].as<unsigned int>(), in.metadata["meta"]["dimensions"][1].as<unsigned int>()} / fornani::constants::u32_chunk_size;
	auto real_dimensions = sf::Vector2u{in.metadata["meta"]["dimensions"][0].as<unsigned int>(), in.metadata["meta"]["dimensions"][1].as<unsigned int>()};
	m_box.setFillColor(room_color_v);
	m_box.setSize(sf::Vector2f{dimensions} * spacing_v);
	m_texture.clear();
	if (!m_texture.resize(real_dimensions) || dimensions.x * dimensions.y == 0.f) { return; }
	auto cell = sf::RectangleShape{};
	m_status == RoomStatus::unfinished ? cell.setFillColor(fornani::colors::pioneer_red) : m_status == RoomStatus::prototype ? cell.setFillColor(fornani::colors::goldenrod) : cell.setFillColor(fornani::colors::green);
	cell.setFillColor(sf::Color{cell.getFillColor().r, cell.getFillColor().g, cell.getFillColor().b, 140});
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
	m_data->metadata["meta"]["minimap"] = has_flag_set(RoomFlags::include_in_minimap);
	m_data->metadata["meta"]["use_template"] = has_flag_set(RoomFlags::use_template);
	m_data->metadata["meta"]["properties"]["interior"] = has_flag_set(RoomFlags::interior);
	m_data->metadata["meta"]["properties"]["day_night_shift"] = has_flag_set(RoomFlags::day_night_shift);
	m_data->metadata["meta"]["metagrid"][0] = m_position.x;
	m_data->metadata["meta"]["metagrid"][1] = m_position.y;
	auto msg = std::string{};
	return m_data->metadata.to_file((svc.finder.paths.levels / std::filesystem::path{m_data->region_label} / std::filesystem::path{m_data->room_label + ".json"}).string());
}

void Room::increment_status(fornani::automa::ServiceProvider& svc) {
	m_status = m_status == RoomStatus::unfinished ? RoomStatus::prototype : m_status == RoomStatus::prototype ? RoomStatus::production : RoomStatus::unfinished;
	m_data->metadata["meta"]["development_status"] = static_cast<int>(m_status);
	auto msg = std::string{};
	if (!m_data->metadata.to_file((svc.finder.paths.levels / std::filesystem::path{m_data->region_label} / std::filesystem::path{m_data->room_label + ".json"}).string())) {}
}

void Room::render(sf::RenderWindow& win, sf::Vector2f cam) {

	// calculate zoom
	sf::View const& view = win.getView();
	float zoom = view.getSize().x / win.getDefaultView().getSize().x;
	float base_thickness = m_highlighted ? -2.f : -1.f;

	m_box.setOutlineThickness(base_thickness * zoom);
	auto h_color = m_status == RoomStatus::unfinished ? fornani::colors::pioneer_red : m_status == RoomStatus::prototype ? fornani::colors::goldenrod : fornani::colors::green;
	m_box.setOutlineColor(h_color);
	m_highlighted ? m_box.setFillColor(sf::Color{h_color.r, h_color.g, h_color.b, 80}) : m_box.setFillColor(sf::Color::Transparent);

	if (no_border) { m_box.setOutlineColor(sf::Color::Transparent); }
	m_box.setOutlineThickness(base_thickness * zoom);
	m_box.setPosition(get_board_position() + cam);
	auto sprite = sf::Sprite{m_texture.getTexture()};
	has_flag_set(RoomFlags::include_in_minimap) ? sprite.setColor(sf::Color::White) : sprite.setColor(fornani::colors::periwinkle);
	sprite.setPosition(m_box.getPosition());
	sprite.scale({spacing_v / fornani::constants::f_chunk_size, spacing_v / fornani::constants::f_chunk_size});
	win.draw(sprite);
	win.draw(m_box);
	if (show_tags) {
		auto tag_barrier = sf::Vector2f{4.f, 4.f};
		auto interior_tag = sf::CircleShape{2.f};
		has_flag_set(RoomFlags::interior) ? interior_tag.setFillColor(fornani::colors::dark_fucshia) : interior_tag.setFillColor(fornani::colors::bright_purple);
		interior_tag.setPosition(m_box.getPosition());
		win.draw(interior_tag);
		has_flag_set(RoomFlags::day_night_shift) ? interior_tag.setFillColor(fornani::colors::bright_orange) : interior_tag.setFillColor(fornani::colors::navy_blue);
		interior_tag.setPosition(m_box.getPosition() + sf::Vector2f{0.f, 6.f});
		win.draw(interior_tag);

		auto status_indicator = sf::CircleShape{8.f};
		status_indicator.setPosition(m_box.getGlobalBounds().getCenter());
		status_indicator.setOrigin({4.f, 4.f});
		m_status == RoomStatus::unfinished	? status_indicator.setFillColor(fornani::colors::pioneer_dark_red)
		: m_status == RoomStatus::prototype ? status_indicator.setFillColor(fornani::colors::goldenrod)
											: status_indicator.setFillColor(fornani::colors::green);
		win.draw(status_indicator);
	}
}

} // namespace pi
