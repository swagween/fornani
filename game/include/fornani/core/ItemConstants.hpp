
#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>

namespace fornani::item {

enum class ItemType : std::uint8_t { ability, key, unique, equippable, collectible, useable, gizmo, apparel, plugin, COUNT };

[[nodiscard]] constexpr auto get_item_table_dimensions(item::ItemType type) -> sf::Vector2i {
	switch (type) {
	case item::ItemType::ability: return {9, 1};
	case item::ItemType::key: return {20, 1};
	case item::ItemType::unique: return {12, 3};
	case item::ItemType::equippable: return {12, 2};
	case item::ItemType::collectible: return {10, 2};
	case item::ItemType::useable: return {4, 1};
	case item::ItemType::gizmo: return {8, 1};
	default: return sf::Vector2i{1, 1};
	}
}

} // namespace fornani::item
