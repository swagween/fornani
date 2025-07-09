
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/InventorySelector.hpp"
#include "fornani/gui/gizmos/DescriptionGizmo.hpp"

namespace fornani::gui {

class InventoryGizmo : public Gizmo {
  public:
	InventoryGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground = false) override;
	bool handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) override;

  private:
	void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;

	int m_max_slots;
	int m_current_item_id{};

	util::RectPath m_path;
	util::RectPath m_lid_path;

	InventorySelector m_selector;
	std::unique_ptr<DescriptionGizmo> m_description;

	sf::Vector2f m_inventory_offset{48.f, 114.f};

	sf::Sprite m_sprite;
	sf::Sprite m_item_sprite;
};

} // namespace fornani::gui
