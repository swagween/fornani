
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/ColorUtils.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::gui {

InventoryWindow::InventoryWindow(automa::ServiceProvider& svc, world::Map& map, player::Player& player)
	: m_cell_dimensions{svc.window->f_screen_dimensions()}, m_dashboard{std::make_unique<Dashboard>(svc, map, player, sf::Vector2f{300.f, 300.f})}, m_camera{.parallax{0.9f}} {
	m_debug.border.setFillColor(sf::Color{12, 12, 20});
	m_debug.border.setSize(svc.window->f_screen_dimensions());
	m_debug.border.setOutlineColor(colors::green);
	m_debug.border.setOutlineThickness(-2.f);
	m_debug.center.setFillColor(colors::red);
	m_debug.center.setRadius(32.f);
	m_debug.center.setOrigin({32.f, 32.f});
	boundary.size = svc.window->f_screen_dimensions() * 3.f;
	boundary.position = -1.f * svc.window->f_screen_dimensions();
	m_background.setFillColor(colors::pioneer_black);
	m_background.setSize(svc.window->f_screen_dimensions());
	m_dashboard->set_position(sf::Vector2f{250.f, 32.f}, true);
	svc.soundboard.flags.console.set(audio::Console::menu_open);
	svc.music_player.pause();
}

void InventoryWindow::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {

	auto& controller = svc.controller_map;

	if (m_view == InventoryView::focused) {
		if (!m_dashboard->handle_inputs(controller, svc.soundboard)) { m_grid_position = {}; }
	}

	m_background.setFillColor(util::ColorUtils::fade_in(colors::pioneer_black));

	if (m_view == InventoryView::dashboard) {
		auto const& up = controller.digital_action_status(config::DigitalAction::menu_up).held;
		auto const& down = controller.digital_action_status(config::DigitalAction::menu_down).held;
		auto const& left = controller.digital_action_status(config::DigitalAction::menu_left).held;
		auto const& right = controller.digital_action_status(config::DigitalAction::menu_right).held;
		auto const& selected = controller.digital_action_status(config::DigitalAction::menu_select).triggered;
		m_dashboard->set_selection({0, 0});
		if (up) { m_dashboard->set_selection({0, -1}); }
		if (down) { m_dashboard->set_selection({0, 1}); }
		if (left) { m_dashboard->set_selection({-1, 0}); }
		if (right) { m_dashboard->set_selection({1, 0}); }
		if (controller.gamepad_connected()) { m_dashboard->set_selection(controller.get_i_joystick_throttle(true)); }
		if (selected && m_dashboard->is_hovering()) {
			if (m_dashboard->get_selected_position().x == 0) { m_grid_position.y = ccm::ext::clamp(m_grid_position.y + m_dashboard->get_selected_position().y, -1.f, 1.f); }
			if (m_dashboard->get_selected_position().y == 0) { m_grid_position.x = ccm::ext::clamp(m_grid_position.x + m_dashboard->get_selected_position().x, -1.f, 1.f); }
			if (m_dashboard->select_gizmo()) {
				m_view = InventoryView::focused;
			} else {
				m_view = InventoryView::dashboard;
				m_grid_position = {};
			}
		}
	}

	auto offset = m_view == InventoryView::dashboard ? sf::Vector2f{m_dashboard->get_selected_position()} * 128.f : sf::Vector2f{};
	auto horizontal_dampen{0.7f}; // we want to display the gizmo's connection to the dashboard for wardrobe and inventory gizmos
	auto target{sf::Vector2f{m_cell_dimensions.x * m_grid_position.x * horizontal_dampen, m_cell_dimensions.y * m_grid_position.y} + offset};
	if (m_dashboard->get_selected_position().x == -1) { target += sf::Vector2f{48.f, 24.f}; } // inventory is slightly lower than other gizmos
	m_camera.steering.seek(m_camera.physics, target, 0.0035f);
	m_camera.physics.simple_update();
	m_dashboard->set_position({250.f, 0.f});
	m_dashboard->update(svc, player, map);

	if (controller.digital_action_status(config::DigitalAction::menu_cancel).triggered) { m_view = m_view == InventoryView::focused ? InventoryView::dashboard : InventoryView::exit; }
	if (controller.digital_action_status(config::DigitalAction::inventory_close).triggered) { m_view = InventoryView::exit; }
	if (exit_requested()) {
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		util::ColorUtils::reset();
	}
}

void InventoryWindow::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player) {
	win.draw(m_background);
	for (auto i{-1}; i < 4; i += 2) {
		for (auto j{-1}; j < 4; j += 2) {
			m_debug.center.setPosition(sf::Vector2f{m_cell_dimensions.x * 0.5f * static_cast<float>(i) - m_camera.physics.position.x, m_cell_dimensions.y * 0.5f * static_cast<float>(j) - m_camera.physics.position.y});
			// win.draw(m_debug.center);
		}
	}
	m_dashboard->render(svc, win, player, m_camera.physics.position);
}

} // namespace fornani::gui
