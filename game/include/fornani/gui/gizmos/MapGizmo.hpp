
#pragma once

#include "fornani/gui/Gizmo.hpp"
#include "fornani/gui/MiniMap.hpp"

namespace fornani::world {
class Map;
}

namespace fornani::gui {

class MapGizmo : public Gizmo {
  public:
	MapGizmo(automa::ServiceProvider& svc, world::Map& map);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) override;
	bool handle_inputs(config::ControllerMap& controller) override;

  private:
	MiniMap m_minimap;
};

} // namespace fornani::gui
