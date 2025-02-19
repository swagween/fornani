
#include "fornani/gui/Dashboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/gizmos/MapGizmo.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::gui {

Dashboard::Dashboard(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f dimensions) : m_physical{.dimensions{dimensions}} {
	m_debug.box.setFillColor(sf::Color{180, 150, 20, 50});
	m_debug.box.setOutlineThickness(-2.f);
	m_debug.box.setOutlineColor(sf::Color{220, 180, 10, 180});
	m_debug.box.setSize(m_physical.dimensions);
	m_debug.box.setOrigin(m_physical.dimensions * 0.5f);
	for (int i = 0; i < 4; ++i) {
		m_debug.buttons.push_back(GizmoButton{});
		m_debug.buttons.back().box.setSize({64.f, 64.f});
		m_debug.buttons.back().box.setFillColor(sf::Color::Transparent);
		m_debug.buttons.back().box.setOutlineThickness(-2.f);
		m_debug.buttons.back().box.setOrigin({32.f, 32.f});
	}

	m_gizmos.push_back(std::make_unique<MapGizmo>(svc, map));
}

void Dashboard::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	auto& controller = svc.controller_map;
	for (auto& gizmo : m_gizmos) { gizmo->update(svc, player, map); }
	m_physical.physics.simple_update();
}

void Dashboard::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {

	// debug stuff
	m_debug.box.setPosition(m_physical.physics.position - cam);
	auto pos{sf::Vector2f{}};
	auto ctr{0};
	auto dist{256.f};
	for (auto& button : m_debug.buttons) {
		pos.x = ctr % 2 == 0 ? 0.f : ctr == 1 ? dist : -dist;
		pos.y = ctr % 2 == 1 ? 0.f : ctr == 2 ? dist * 0.5f : -dist * 0.5f;
		switch (ctr) { // up, right, down, left
		case 0: button.position = {0, -1}; break;
		case 1: button.position = {1, 0}; break;
		case 2: button.position = {0, 1}; break;
		case 3: button.position = {-1, 0}; break;
		}
		button.state = button.position == m_selected_position ? GizmoButtonState::hovered : GizmoButtonState::off;
		if (button.state == GizmoButtonState::hovered) { m_current_gizmo = std::clamp(ctr, 0, static_cast<int>(m_gizmos.size() - 1)); }
		button.box.setPosition(svc.constants.f_center_screen + pos);
		button.state == GizmoButtonState::hovered ? button.box.setOutlineColor(svc.styles.colors.bright_orange) : button.box.setOutlineColor(svc.styles.colors.dark_fucshia);
		// win.draw(button.box);
		++ctr;
	}

	// real renders
	for (auto& gizmo : m_gizmos) { gizmo->render(svc, win, cam); }
}

bool Dashboard::handle_inputs(config::ControllerMap& controller) { return m_gizmos.at(m_current_gizmo)->handle_inputs(controller); }

void Dashboard::set_position(sf::Vector2f to_position, bool force) {
	if (force) {
		m_physical.physics.position = to_position;
	} else {
		m_physical.steering.seek(m_physical.physics, to_position, 0.001f);
	}
}

void Dashboard::set_selection(sf::Vector2i to_selection) { m_selected_position = to_selection; }

void Dashboard::hover(sf::Vector2i direction) {}

} // namespace fornani::gui
