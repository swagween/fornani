
#include "fornani/entities/item/Item.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::item {

Item::Item(dj::Json& source, std::string_view label) : m_label{label} {
	auto const& in_data = source[label];
	m_id = in_data["id"].as<int>();
	m_type = static_cast<ItemType>(in_data["category"].as<int>());
	m_lookup.position.x = in_data["lookup"][0].as<int>();
	m_lookup.position.y = in_data["lookup"][1].as<int>();
	m_table_origin.x = in_data["origin"][0].as<int>();
	m_table_origin.y = in_data["origin"][1].as<int>();
	m_lookup.position = m_lookup.position.componentWiseMul(constants::i_resolution_vec);
	m_lookup.size = constants::i_resolution_vec;

	if (in_data["readable"].as_bool()) { m_flags.set(ItemFlags::readable); }

	m_info.actual_title = in_data["actual_title"].as_string().data();
	m_info.actual_description = in_data["actual_description"].as_string().data();
	m_info.naive_title = in_data["naive_title"] ? in_data["naive_title"].as_string().data() : m_info.actual_title;
	m_info.naive_description = in_data["naive_description"] ? in_data["naive_description"].as_string().data() : m_info.actual_description;

	m_stats.value = in_data["value"].as<int>();
	m_stats.rarity = static_cast<Rarity>(in_data["rarity"].as<int>());
}

void Item::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position) {
	sprite.setTextureRect(m_lookup);
	sprite.setPosition(position);
	win.draw(sprite);
}

void Item::reveal() { m_state.set(ItemState::revealed); }

std::vector<std::string> Item::generate_menu_list() const {
	auto ret = std::vector<std::string>();
	if (m_flags.test(ItemFlags::readable)) { ret.push_back("read"); }
	ret.push_back("cancel");
	return ret;
}

} // namespace fornani::item
