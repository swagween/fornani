
#pragma once

#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"

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

enum class ItemType : std::uint8_t { key, collectible, apparel };
enum class ItemFlags : std::uint8_t { sellable, vendor_spawnable, gizmo, ability };

class Item {
  public:
	Item(dj::Json& source, std::string_view label, ItemType type);
	~Item() = default;

	virtual void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f position);
	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_type() const -> ItemType { return m_type; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto get_lookup() const -> sf::IntRect { return m_lookup; }
	[[nodiscard]] auto get_table_position() const -> sf::Vector2f { return sf::Vector2f{static_cast<float>(m_lookup.position.x), static_cast<float>(m_lookup.position.y)}; }

  protected:
	int m_id{};
	std::string m_label;
	ItemInformation m_info{};
	ItemType m_type;
	util::BitFlags<ItemFlags> m_flags{};

  private:
	sf::IntRect m_lookup{};

	io::Logger m_logger{"item"};
};

} // namespace fornani::item
