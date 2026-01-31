
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <optional>
#include <string_view>

namespace fornani::item {

struct ItemInformation {
	std::string naive_title{};
	std::string actual_title{};
	std::string naive_description{};
	std::string actual_description{};
};

struct ItemStats {
	int value{};
	Rarity rarity{};
	int apparel_type{};
	int stack_limit{1};
};

enum class ItemType { ability, key, collectible, gizmo, apparel };
enum class ItemFlags { sellable, readable, equippable, wearable, invisible };
enum class ItemState { revealed, equipped };

class Item : public Polymorphic {
  public:
	Item(dj::Json const& source, std::string_view label);

	virtual void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position);

	void reveal();
	void set_equipped(bool to);
	std::vector<std::string> generate_menu_list(dj::Json const& in) const;

	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_type() const -> ItemType { return m_type; }
	[[nodiscard]] auto get_rarity() const -> Rarity { return m_stats.rarity; }
	[[nodiscard]] auto get_value() const -> int { return m_stats.value; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto get_title() const -> std::string { return is_revealed() ? m_info.actual_title : m_info.naive_title; }
	[[nodiscard]] auto get_description() const -> std::string { return is_revealed() ? m_info.actual_description : m_info.naive_description; }
	[[nodiscard]] auto get_lookup() const -> sf::IntRect { return m_lookup; }
	[[nodiscard]] auto get_table_index(int table_width) const -> int { return m_table_origin.x + m_table_origin.y * table_width; }
	[[nodiscard]] auto get_origin() const -> sf::Vector2i { return m_table_origin; }
	[[nodiscard]] auto get_f_origin() const -> sf::Vector2f { return sf::Vector2f{m_table_origin}; }
	[[nodiscard]] auto get_table_position() const -> sf::Vector2f { return sf::Vector2f{m_lookup.position} - get_f_origin(); }
	[[nodiscard]] auto get_apparel_type() const -> std::optional<int> { return m_stats.apparel_type != -1 ? std::optional<int>{m_stats.apparel_type} : std::nullopt; }

	[[nodiscard]] auto is_sellable() const -> bool { return m_flags.test(ItemFlags::sellable); }
	[[nodiscard]] auto is_revealed() const -> bool { return m_state.test(ItemState::revealed); }
	[[nodiscard]] auto is_readable() const -> bool { return m_flags.test(ItemFlags::readable); }
	[[nodiscard]] auto is_equippable() const -> bool { return m_flags.test(ItemFlags::equippable); }
	[[nodiscard]] auto is_wearable() const -> bool { return m_flags.test(ItemFlags::wearable); }
	[[nodiscard]] auto is_invisible() const -> bool { return m_flags.test(ItemFlags::invisible); }

	[[nodiscard]] auto is_key() const -> bool { return m_type == ItemType::key; }
	[[nodiscard]] auto is_gizmo() const -> bool { return m_type == ItemType::gizmo; }
	[[nodiscard]] auto is_apparel() const -> bool { return m_type == ItemType::apparel; }
	[[nodiscard]] auto is_collectible() const -> bool { return m_type == ItemType::collectible; }
	[[nodiscard]] auto is_unique() const -> bool { return m_stats.stack_limit == 1; }
	[[nodiscard]] auto is_ability() const -> bool { return m_type == ItemType::ability; }

  protected:
	int m_id{};
	sf::Vector2i m_table_origin{};
	std::string m_label;
	ItemInformation m_info{};
	ItemStats m_stats{};
	ItemType m_type;
	util::BitFlags<ItemFlags> m_flags{};
	util::BitFlags<ItemState> m_state{};

  private:
	sf::IntRect m_lookup{};

	io::Logger m_logger{"item"};
};

} // namespace fornani::item
