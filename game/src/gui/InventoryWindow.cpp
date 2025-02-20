
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc, world::Map& map)
	: m_cell_dimensions{svc.constants.f_screen_dimensions}, m_dashboard{std::make_unique<Dashboard>(svc, map, sf::Vector2f{300.f, 300.f})}, m_camera{.parallax{0.9f}}, m_debug{.sprite{sf::Sprite{svc.assets.t_inv_test}}} {
	m_debug.border.setFillColor(sf::Color{12, 12, 20});
	m_debug.border.setSize(svc.constants.f_screen_dimensions);
	m_debug.border.setOutlineColor(svc.styles.colors.green);
	m_debug.border.setOutlineThickness(-2.f);
	m_debug.center.setFillColor(svc.styles.colors.red);
	m_debug.center.setRadius(32.f);
	m_debug.center.setOrigin({32.f, 32.f});
	boundary.size = svc.constants.f_screen_dimensions * 3.f;
	boundary.position = -1.f * svc.constants.f_screen_dimensions;
	m_background.setFillColor(svc.styles.colors.pioneer_black);
	m_background.setSize(svc.constants.f_screen_dimensions);
	m_dashboard->set_position(sf::Vector2f{250.f, 64.f}, true);
	svc.soundboard.flags.console.set(audio::Console::menu_open);
}

void InventoryWindow::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {

	auto& controller = svc.controller_map;

	if (m_view == InventoryView::focused) {
		if (!m_dashboard->handle_inputs(controller)) {
			m_grid_position = {};
			m_dashboard->set_selection({});
		}
	}

	if (m_view == InventoryView::dashboard) {
		if (controller.digital_action_status(config::DigitalAction::menu_up).triggered) { m_dashboard->set_selection({0, -1}); }
		if (controller.digital_action_status(config::DigitalAction::menu_down).triggered) { m_dashboard->set_selection({0, 1}); }
		if (controller.digital_action_status(config::DigitalAction::menu_left).triggered) { m_dashboard->set_selection({-1, 0}); }
		if (controller.digital_action_status(config::DigitalAction::menu_right).triggered) { m_dashboard->set_selection({1, 0}); }
		if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) {
			if (m_dashboard->get_selected_position().x == 0) { m_grid_position.y = std::clamp(m_grid_position.y + m_dashboard->get_selected_position().y, -1.f, 1.f); }
			if (m_dashboard->get_selected_position().y == 0) { m_grid_position.x = std::clamp(m_grid_position.x + m_dashboard->get_selected_position().x, -1.f, 1.f); }
			m_view = InventoryView::focused;
		}
	}

	auto offset = m_view == InventoryView::dashboard ? sf::Vector2f{m_dashboard->get_selected_position()} * 128.f : sf::Vector2f{};
	auto target{sf::Vector2f{m_cell_dimensions.x * m_grid_position.x, m_cell_dimensions.y * m_grid_position.y} + offset};
	m_camera.steering.seek(m_camera.physics, target, 0.003f);
	m_camera.physics.simple_update();
	m_dashboard->set_position({250.f, 0.f});
	m_dashboard->update(svc, player, map);

	if (controller.digital_action_status(config::DigitalAction::menu_cancel).triggered) { m_view = m_view == InventoryView::focused ? InventoryView::dashboard : InventoryView::exit; }
	if (exit_requested()) { svc.soundboard.flags.menu.set(audio::Menu::backward_switch); }
}

void InventoryWindow::render(automa::ServiceProvider& svc, sf::RenderWindow& win) {
	m_debug.border.setPosition(-m_camera.physics.position);
	win.draw(m_background);
	// win.draw(m_debug.border);
	for (auto i{-1}; i < 4; i += 2) {
		for (auto j{-1}; j < 4; j += 2) {
			m_debug.center.setPosition(sf::Vector2f{m_cell_dimensions.x * 0.5f * static_cast<float>(i) - m_camera.physics.position.x, m_cell_dimensions.y * 0.5f * static_cast<float>(j) - m_camera.physics.position.y});
			// win.draw(m_debug.center);
		}
	}

	m_debug.sprite.setOrigin({582.f, 378.f});
	m_debug.sprite.setScale({2.f, 2.f});
	m_debug.sprite.setPosition(m_dashboard->get_position() - m_camera.physics.position);
	// win.draw(m_debug.sprite);
	m_dashboard->render(svc, win, m_camera.physics.position);
}

} // namespace fornani::gui
