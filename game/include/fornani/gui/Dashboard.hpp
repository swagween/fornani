
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/gui/Gizmo.hpp"
#include "fornani/utils/RectPath.hpp"

#include <SFML/Graphics.hpp>

#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::config {
class ControllerMap;
}

namespace fornani::gui {

enum class GizmoButtonState { off, hovered, clicked };

struct GizmoButton {
	sf::RectangleShape box{};
	sf::Vector2i position{};
	GizmoButtonState state{};
};

class Dashboard {
  public:
	Dashboard(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f dimensions);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	bool handle_inputs(config::ControllerMap& controller);
	void set_position(sf::Vector2f to_position, bool force = false);
	void set_selection(sf::Vector2i to_selection);
	void select_gizmo();
	void hover(sf::Vector2i direction);
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_physical.physics.position; }
	[[nodiscard]] auto get_selected_position() const -> sf::Vector2i { return m_selected_position; }

  private:
	std::vector<std::unique_ptr<Gizmo>> m_gizmos{};
	sf::Vector2i m_selected_position{};
	int m_current_gizmo{};

	sf::Sprite m_sprite;

	struct {
		Constituent top_left_frontplate;
		Constituent top_right_frontplate;
		Constituent arsenal_frontplate;
		Constituent top_left_slot;
		Constituent top_right_slot;
		Constituent arsenal_slot;
		Constituent motherboard;
	} m_constituents{};

	struct {
		util::RectPath map;
	} m_paths;

	struct {
		components::SteeringBehavior steering{};
		components::PhysicsComponent physics{};
		sf::Vector2f dimensions{};
	} m_physical{};

	struct {
		sf::RectangleShape box{};
		std::vector<GizmoButton> buttons{};
	} m_debug{};
};

} // namespace fornani::gui
