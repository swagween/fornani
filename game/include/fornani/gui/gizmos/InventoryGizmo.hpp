
#pragma once

#include <fornani/entities/item/Item.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/OrbDisplay.hpp>
#include <optional>
#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/InventorySelector.hpp"
#include "fornani/gui/gizmos/DescriptionGizmo.hpp"

namespace fornani::gui {

enum class InventoryGizmoFlags : std::uint8_t { is_item_hovered };

class InventoryGizmo : public Gizmo {
  public:
	InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;
	[[nodiscard]] auto is_item_hovered() const -> int { return m_flags.test(InventoryGizmoFlags::is_item_hovered); }

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;

	void handle_menu_selection(int selection);

	int m_max_slots;
	int m_current_item_lookup{};
	int m_current_item_id{};

	util::RectPath m_path;
	util::RectPath m_lid_path;

	InventorySelector m_selector;
	std::unique_ptr<DescriptionGizmo> m_description;
	OrbDisplay m_orb_display;
	std::optional<MiniMenu> m_item_menu{};

	sf::Vector2f m_inventory_offset;

	sf::Sprite m_sprite;
	sf::Sprite m_item_sprite;

	util::BitFlags<InventoryGizmoFlags> m_flags{};

	item::Item* m_current_item;
	automa::ServiceProvider* m_services;
};

} // namespace fornani::gui
