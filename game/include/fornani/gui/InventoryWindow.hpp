
#pragma once
#include "Console.hpp"
#include "MiniMap.hpp"
#include "MiniMenu.hpp"
#include "Selector.hpp"
#include "WardrobeWidget.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/io/Logger.hpp"

namespace fornani::player {
class Player;
}

namespace fornani::item {
class Item;
}

namespace fornani::gui {

class InventoryWindow {
  public:
	InventoryWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

  private:
	sf::FloatRect boundary{};
	sf::Vector2f m_grid_position{};
	sf::Vector2f m_cell_dimensions{};
	struct {
		components::SteeringBehavior steering{};
		components::PhysicsComponent physics{};
	} m_camera{};
	struct {
		sf::RectangleShape border{};
		sf::RectangleShape boundary{};
		sf::CircleShape center{};
	} m_debug{};
	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
