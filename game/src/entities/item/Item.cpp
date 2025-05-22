
#include "fornani/entities/item/Item.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::item {

Item::Item(dj::Json& source, std::string_view label, ItemType type) : m_label{label}, m_type{type} {
	auto& in_data{source[label]};
	m_id = in_data["id"].as<int>();
	m_lookup.position.x = in_data["lookup"][0].as<int>();
	m_lookup.position.y = in_data["lookup"][1].as<int>();
	m_table_origin.x = in_data["origin"][0].as<int>();
	m_table_origin.y = in_data["origin"][1].as<int>();
	m_table_position = {in_data["lookup"][0].as<float>() - in_data["origin"][0].as<float>(), in_data["lookup"][1].as<float>() - in_data["origin"][1].as<float>()};
	m_lookup.position = m_lookup.position.componentWiseMul(constants::i_resolution_vec);
	m_lookup.size = constants::i_resolution_vec;

	m_info.actual_title = in_data["actual_title"].as_string();
	m_info.actual_description = in_data["actual_description"].as_string();
	m_info.naive_title = in_data["naive_title"] ? in_data["naive_title"].as_string() : m_info.actual_title;
	m_info.naive_description = in_data["naive_description"] ? in_data["naive_description"].as_string() : m_info.actual_description;
}

void Item::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position) {
	sprite.setTextureRect(m_lookup);
	sprite.setPosition(position);
	win.draw(sprite);
}

void Item::reveal() { m_state.set(ItemState::revealed); }

} // namespace fornani::item
