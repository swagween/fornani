
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc) : m_cell_dimensions{svc.constants.f_screen_dimensions} {
	m_debug.border.setFillColor(sf::Color{12, 12, 20});
	m_debug.border.setSize(svc.constants.f_screen_dimensions);
	m_debug.border.setOutlineColor(svc.styles.colors.green);
	m_debug.border.setOutlineThickness(-2.f);
	m_debug.center.setFillColor(svc.styles.colors.red);
	m_debug.center.setRadius(32.f);
	m_debug.center.setOrigin({32.f, 32.f});
	boundary.size = svc.constants.f_screen_dimensions * 3.f;
	boundary.position = -1.f * svc.constants.f_screen_dimensions;
	m_debug.boundary.setFillColor(svc.styles.colors.ui_black);
	m_debug.boundary.setOutlineColor(svc.styles.colors.blue);
	m_debug.boundary.setOutlineThickness(-16.f);
	m_debug.boundary.setSize(boundary.size);
	m_debug.boundary.setPosition(boundary.position);
}

void InventoryWindow::update(automa::ServiceProvider& svc) {
	auto& controller = svc.controller_map;
	if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) { m_grid_position.y = std::clamp(m_grid_position.y - 1.f, -1.f, 1.f); }
	if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) { m_grid_position.y = std::clamp(m_grid_position.y + 1.f, -1.f, 1.f); }
	if (controller.digital_action_status(config::DigitalAction::menu_left).triggered) { m_grid_position.x = std::clamp(m_grid_position.x - 1.f, -1.f, 1.f); }
	if (controller.digital_action_status(config::DigitalAction::menu_right).triggered) { m_grid_position.x = std::clamp(m_grid_position.x + 1.f, -1.f, 1.f); }
	m_camera.steering.seek(m_camera.physics, sf::Vector2f{m_cell_dimensions.x * m_grid_position.x, m_cell_dimensions.y * m_grid_position.y}, 0.002f);
	m_camera.physics.simple_update();
}

void InventoryWindow::render(sf::RenderWindow& win) {
	m_debug.boundary.setPosition(-m_cell_dimensions - m_camera.physics.position);
	m_debug.border.setPosition(-m_camera.physics.position);
	win.draw(m_debug.boundary);
	win.draw(m_debug.border);
	for (auto i{-1}; i < 4; i += 2) {
		for (auto j{-1}; j < 4; j += 2) {
			m_debug.center.setPosition(sf::Vector2f{m_cell_dimensions.x * 0.5f * static_cast<float>(i) - m_camera.physics.position.x, m_cell_dimensions.y * 0.5f * static_cast<float>(j) - m_camera.physics.position.y});
			win.draw(m_debug.center);
		}
	}
}

} // namespace fornani::gui
