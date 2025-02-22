
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
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

struct Constituent {
	sf::IntRect lookup{};
	sf::Vector2f position{};
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam) const;
};

class Gizmo {
  public:
	explicit Gizmo(std::string const& label) : m_label(label) {}
	virtual ~Gizmo() = default;
	virtual void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	virtual bool handle_inputs(config::ControllerMap& controller);
	void select() { m_state = GizmoState::selected; }
	void deselect() { m_state = GizmoState::hovered; }

  protected:
	std::string m_label{};
	GizmoState m_state{};
	// the actual position of the gizmo, will generally target m_placement
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	// the (x, y) of the target top left position of gizmo. all constituent parts will refer to this position for drawing.
	sf::Vector2f m_placement{};
	//
	io::Logger m_logger{"Gizmo"};
};

} // namespace fornani::gui
