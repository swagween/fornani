
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/io/Logger.hpp"

#include <SFML/Graphics.hpp>

#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::config {
class ControllerMap;
}

namespace fornani::gui {

enum class GizmoState { neutral, hovered, selected };

class Gizmo {
  public:
	explicit Gizmo(std::string const& label) : m_label(label) {}
	virtual ~Gizmo() = default;
	virtual void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	virtual bool handle_inputs(config::ControllerMap& controller);

  protected:
	std::string m_label{};

	// the actual position of the gizmo, will generally target m_placement
	components::PhysicsComponent m_physics{};
	// the (x, y) of the target top left position of gizmo. all constituent parts will refer to this position for drawing.
	sf::Vector2f m_placement{};
	//
	io::Logger m_logger{"Gizmo"};
};

} // namespace fornani::gui
