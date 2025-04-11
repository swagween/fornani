
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Polymorphic.hpp"

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <string_view>

namespace fornani::item {

struct ItemInformation {
	std::string naive_title{};
	std::string actual_title{};
	std::string naive_description{};
	std::string actual_description{};
};

enum class ItemType : std::uint8_t { key, apparel, collectible };
enum class ItemFlags : std::uint8_t { sellable, vendor_spawnable, gizmo, ability };
enum class ItemState : std::uint8_t { revealed };

class Item : public Polymorphic {
  public:
	Item(dj::Json& source, std::string_view label, ItemType type);

	virtual void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position);

	void reveal();

	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_type() const -> ItemType { return m_type; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto get_title() const -> std::string { return is_revealed() ? m_info.actual_title : m_info.naive_title; }
	[[nodiscard]] auto get_description() const -> std::string { return is_revealed() ? m_info.actual_description : m_info.naive_description; }
	[[nodiscard]] auto get_lookup() const -> sf::IntRect { return m_lookup; }
	[[nodiscard]] auto get_table_position() const -> sf::Vector2f { return m_table_position; }

	[[nodiscard]] auto is_revealed() const -> bool { return m_state.test(ItemState::revealed); }

  protected:
	int m_id{};
	sf::Vector2i m_table_origin{};
	sf::Vector2f m_table_position{};
	std::string m_label;
	ItemInformation m_info{};
	ItemType m_type;
	util::BitFlags<ItemFlags> m_flags{};
	util::BitFlags<ItemState> m_state{};

  private:
	sf::IntRect m_lookup{};

	io::Logger m_logger{"item"};
};

} // namespace fornani::item
