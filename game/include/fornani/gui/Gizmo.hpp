
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/RectPath.hpp"

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
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) const;
};

class Gizmo {
  public:
	explicit Gizmo(std::string const& label, bool foreground) : m_label(label), m_foreground(foreground) {}
	virtual ~Gizmo() = default;
	virtual void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool foreground = false);
	virtual bool handle_inputs(config::ControllerMap& controller);
	void select();
	void deselect();
	[[nodiscard]] auto is_foreground() const -> bool { return m_foreground; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }

  protected:
	bool m_switched{};
	bool m_foreground{};
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
