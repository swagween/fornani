
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/gui/Dashboard.hpp"
#include "fornani/gui/InventorySelector.hpp"
#include "fornani/gui/MiniMenu.hpp"
#include "fornani/io/Logger.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::item {
class Item;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {

enum class InventoryView { dashboard, focused, exit };

class InventoryWindow {
  public:
	InventoryWindow(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player);
	[[nodiscard]] auto exit_requested() const -> bool { return m_exit.is_almost_complete(); }

  private:
	sf::FloatRect boundary{};
	sf::Vector2f m_grid_position{};
	sf::Vector2f m_cell_dimensions{};
	sf::RectangleShape m_background{};
	std::unique_ptr<Dashboard> m_dashboard{};
	InventoryView m_view{};
	util::Cooldown m_exit;

	struct {
		components::SteeringBehavior steering{};
		components::PhysicsComponent physics{};
		float parallax{};
	} m_camera{};

	struct {
		sf::RectangleShape border{};
		sf::CircleShape center{};
	} m_debug;

	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
