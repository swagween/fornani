
#include <fornani/entities/item/Item.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani::item {

Item::Item(dj::Json const& source, std::string_view label) : m_label{label}, m_type{ItemType::key} {

	auto const& arr = source.as_array();
	auto it = std::find_if(arr.begin(), arr.end(), [&](auto const& v) { return v["tag"].as_string() == label; });

	if (it == arr.end()) {
		NANI_LOG_ERROR(m_logger, "Failed to find item with label {} in item.json!", label);
		return;
	}
	std::size_t id = std::distance(arr.begin(), it);
	auto const& in_data = *it;

	m_id = id;
	m_type = static_cast<ItemType>(in_data["category"].as<int>());
	m_lookup.position.x = in_data["lookup"][0].as<int>();
	m_lookup.position.y = in_data["lookup"][1].as<int>();
	m_table_origin.x = in_data["origin"][0].as<int>();
	m_table_origin.y = in_data["origin"][1].as<int>();
	m_lookup.position = m_lookup.position.componentWiseMul(constants::i_resolution_vec);
	m_lookup.size = constants::i_resolution_vec;

	if (in_data["sellable"].as_bool()) { m_flags.set(ItemFlags::sellable); }
	if (in_data["readable"].as_bool()) { m_flags.set(ItemFlags::readable); }
	if (in_data["equippable"].as_bool()) { m_flags.set(ItemFlags::equippable); }
	if (in_data["wearable"].as_bool()) { m_flags.set(ItemFlags::wearable); }
	if (in_data["invisible"].as_bool()) { m_flags.set(ItemFlags::invisible); }
	m_stats.stack_limit = in_data["stack_limit"] ? in_data["stack_limit"].as<int>() : 1;
	if (m_type == ItemType::collectible) { m_stats.stack_limit = 99; }

	m_info.actual_title = in_data["actual_title"].as_string().data();
	m_info.actual_description = in_data["actual_description"].as_string().data();
	m_info.naive_title = in_data["naive_title"] ? in_data["naive_title"].as_string().data() : m_info.actual_title;
	m_info.naive_description = in_data["naive_description"] ? in_data["naive_description"].as_string().data() : m_info.actual_description;

	m_stats.value = in_data["value"].as<int>();
	m_stats.rarity = static_cast<Rarity>(in_data["rarity"].as<int>());
	m_stats.apparel_type = in_data["apparel_type"].is_number() ? in_data["apparel_type"].as<int>() : -1;
}

void Item::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position) {
	sprite.setTextureRect(m_lookup);
	sprite.setPosition(position);
	win.draw(sprite);
}

void Item::reveal() { m_state.set(ItemState::revealed); }

void Item::set_equipped(bool to) { to ? m_state.set(ItemState::equipped) : m_state.reset(ItemState::equipped); }

std::vector<std::string> Item::generate_menu_list(dj::Json const& in) const {
	auto ret = std::vector<std::string>();
	if (m_flags.test(ItemFlags::equippable)) { m_state.test(ItemState::equipped) ? ret.push_back(in["unequip"].as_string()) : ret.push_back(in["equip"].as_string()); }
	if (m_flags.test(ItemFlags::readable)) { ret.push_back(in["read"].as_string()); }
	ret.push_back(in["cancel"].as_string());
	return ret;
}

} // namespace fornani::item
