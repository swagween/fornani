
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/gui/Gizmo.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/utils/RectPath.hpp>
#include <vector>

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

namespace fornani::input {
class InputSystem;
}

namespace fornani::gui {

enum class DashboardState { home, hovering, gizmo };
enum class GizmoButtonState { off, hovered, clicked };

struct GizmoButton {
	sf::RectangleShape box{};
	sf::Vector2i position{};
	GizmoButtonState state{};
};

class Dashboard {
  public:
	Dashboard(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f dimensions);
	void update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2f cam, LightShader& shader);
	bool handle_inputs(input::InputSystem& controller, audio::Soundboard& soundboard);
	void set_position(sf::Vector2f to_position, bool force = false);
	void set_selection(sf::Vector2i to_selection, bool gamepad = false);
	void close();
	///@returns false if gizmo does not exist
	bool select_gizmo();
	void hover(sf::Vector2i direction);
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_physical.physics.position; }
	[[nodiscard]] auto get_selected_position() const -> sf::Vector2i { return m_selected_position; }
	[[nodiscard]] auto is_hovering() const -> bool { return m_state == DashboardState::hovering; }
	[[nodiscard]] auto is_home() const -> bool { return m_state == DashboardState::home; }

  private:
	std::vector<std::unique_ptr<Gizmo>> m_gizmos{};
	sf::Vector2i m_selected_position{};
	DashboardPort m_current_port{};
	DashboardState m_state{};

	Palette m_palette;
	sf::Sprite m_sprite;
	util::Cooldown m_light_shift{light_shift_time_v};
	util::Cooldown m_light_up{light_shift_time_v};

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
		util::RectPath rotary;
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

	automa::ServiceProvider* m_services;

	io::Logger m_logger{"GUI"};
};

} // namespace fornani::gui
