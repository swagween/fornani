
#pragma once

#include <fornani/entities/item/Item.hpp>
#include <fornani/gui/Gizmo.hpp>
#include <fornani/gui/InventorySelector.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/NumberDisplay.hpp>
#include <fornani/gui/OrbDisplay.hpp>
#include <fornani/gui/gizmos/DescriptionGizmo.hpp>
#include <optional>

namespace fornani::gui {

enum class InventoryGizmoFlags { is_item_hovered };
enum class InventoryZoneType { ability, key, collectible, gizmo, COUNT };

struct InventoryZone {
	sf::Vector2i table_dimensions{};
	sf::Vector2f cell_size{};
	sf::Vector2f render_offset{};
};

class InventoryGizmo : public Gizmo {
  public:
	InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;
	[[nodiscard]] auto is_item_hovered() const -> int { return m_flags.test(InventoryGizmoFlags::is_item_hovered); }
	[[nodiscard]] auto get_zone_type() const -> InventoryZoneType { return static_cast<InventoryZoneType>(m_zone_iterator.get()); }

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;

	void handle_menu_selection(player::Player& player, int selection);
	void switch_zones(int modulation);
	void write_description(item::Item& piece, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam);

	[[nodiscard]] auto zone_match(item::ItemType type) const -> bool { return static_cast<InventoryZoneType>(type) == get_zone_type(); }

	std::array<InventoryZone, static_cast<int>(InventoryZoneType::COUNT)> m_zones;
	std::array<sf::Vector2i, static_cast<int>(InventoryZoneType::COUNT)> m_remembered_locations{};
	util::Circuit m_zone_iterator{static_cast<int>(InventoryZoneType::COUNT), static_cast<int>(InventoryZoneType::key)};

	int m_current_item_lookup{};

	bool m_just_switched{};

	util::RectPath m_path;
	util::RectPath m_lid_path;

	std::unique_ptr<InventorySelector> m_selector;
	std::unique_ptr<DescriptionGizmo> m_description;
	OrbDisplay m_orb_display;
	std::vector<NumberDisplay> m_number_displays{};
	std::optional<MiniMenu> m_item_menu{};

	sf::Sprite m_sprite;
	sf::Sprite m_item_sprite;

	sf::Vector2f m_equipped_items_position;

	player::Player* m_player;

	util::BitFlags<InventoryGizmoFlags> m_flags{};

	std::optional<int> m_current_item{};
	automa::ServiceProvider* m_services;
};

} // namespace fornani::gui
