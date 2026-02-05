
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <fornani/utils/RectPath.hpp>
#include <string>

namespace fornani {
class LightShader;
class Palette;
} // namespace fornani

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::input {
class InputSystem;
}

namespace fornani::audio {
class Soundboard;
}

namespace fornani::gui {

constexpr auto light_shift_time_v = 24;

enum class GizmoState { neutral, hovered, selected, closed };
enum class DashboardPort { minimap, wardrobe, arsenal, inventory, invalid };

struct Constituent {
	sf::IntRect lookup{};
	sf::Vector2f position{};
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin, LightShader& shader, Palette& palette) const;
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) const;
};

// essentially a non-selectable gizmo
struct FreeConstituent {
	Constituent constituent{};
	components::PhysicsComponent physics{};
	components::SteeringBehavior steering{};
	void update();
};

class Gizmo : public UniquePolymorphic {
  public:
	explicit Gizmo(std::string const& label, bool foreground) : m_label(label), m_foreground(foreground) {}
	virtual void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position);
	virtual void render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground = false);
	virtual bool handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard);

	void close();
	void select();
	void deselect();
	void neutralize();
	void hover();

	[[nodiscard]] auto is_foreground() const -> bool { return m_foreground; }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto is_neutral() const -> bool { return m_state == GizmoState::neutral; }
	[[nodiscard]] auto is_hovered() const -> bool { return m_state == GizmoState::hovered; }
	[[nodiscard]] auto is_selected() const -> bool { return m_state == GizmoState::selected; }
	[[nodiscard]] auto is_closed() const -> bool { return m_state == GizmoState::closed; }
	[[nodiscard]] auto get_dashboard_port() const -> DashboardPort { return m_dashboard_port ? *m_dashboard_port : DashboardPort::invalid; }

	// debug
	void report();

  protected:
	virtual void on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map);
	virtual void on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map);
	bool m_switched{};
	bool m_foreground{};
	bool m_exit_trigger{};
	util::Cooldown m_light_shift{light_shift_time_v};
	std::string m_label{};
	GizmoState m_state{};
	std::optional<DashboardPort> m_dashboard_port{}; // determines the index in the dashboard's gizmo vector
	// the actual position of the gizmo, will generally target m_placement
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	// the (x, y) of the target top left position of gizmo. all constituent parts will refer to this position for drawing.
	sf::Vector2f m_placement{};
	//
	io::Logger m_logger{"Gizmo"};
};

} // namespace fornani::gui
