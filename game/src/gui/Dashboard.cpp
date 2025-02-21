
#include "fornani/gui/Dashboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/gizmos/ClockGizmo.hpp"
#include "fornani/gui/gizmos/MapGizmo.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

namespace fornani::gui {

Dashboard::Dashboard(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f dimensions)
	: m_physical{.dimensions{dimensions}}, m_sprite{svc.assets.t_dashboard}, m_rects{.top_left{{}, {153, 124}}, .top_right{{153, 0}, {64, 127}}, .bottom{{26, 127}, {184, 137}}, .base{{217, 0}, {222, 212}}} {
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

	// populate dashboard depending on the player's inventory
	auto const& items = svc.data.get_player_items();
	for (auto& i : items.array_view()) {
		if (i["id"].as<int>() == 16) { m_gizmos.push_back(std::make_unique<MapGizmo>(svc, map)); }
	}
	m_gizmos.push_back(std::make_unique<ClockGizmo>(svc, map)); // have to stick this in the for loop once we have a clock item
	m_sprite.setScale(svc.constants.texture_scale);
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
		if (button.state == GizmoButtonState::hovered && m_gizmos.size() > 0) { m_current_gizmo = std::clamp(ctr, 0, static_cast<int>(m_gizmos.size() - 1)); }
		button.box.setPosition(svc.constants.f_center_screen + pos);
		button.state == GizmoButtonState::hovered ? button.box.setOutlineColor(svc.styles.colors.bright_orange) : button.box.setOutlineColor(svc.styles.colors.dark_fucshia);
		win.draw(button.box);
		++ctr;
	}

	// real renders
	for (auto& gizmo : m_gizmos) { gizmo->render(svc, win, cam); }
	m_sprite.setTextureRect(m_rects.base);
	m_sprite.setPosition(m_physical.physics.position + sf::Vector2f{14, 68} - cam);
	win.draw(m_sprite);
	m_sprite.setTextureRect(m_rects.top_left);
	m_sprite.setPosition(m_physical.physics.position - cam);
	win.draw(m_sprite);
	m_sprite.setTextureRect(m_rects.top_right);
	m_sprite.setPosition(m_physical.physics.position + sf::Vector2f{290, 0} - cam);
	win.draw(m_sprite);
	m_sprite.setTextureRect(m_rects.bottom);
	m_sprite.setPosition(m_physical.physics.position + sf::Vector2f{52, 218} - cam);
	win.draw(m_sprite);
}

bool Dashboard::handle_inputs(config::ControllerMap& controller) {
	if (m_gizmos.empty() || m_gizmos.size() <= m_current_gizmo) { return false; }
	return m_gizmos.at(m_current_gizmo)->handle_inputs(controller);
}

void Dashboard::set_position(sf::Vector2f to_position, bool force) {
	if (force) {
		m_physical.physics.position = to_position;
	} else {
		m_physical.steering.seek(m_physical.physics, to_position, 0.002f);
	}
}

void Dashboard::set_selection(sf::Vector2i to_selection) { m_selected_position = to_selection; }

void Dashboard::hover(sf::Vector2i direction) {}

} // namespace fornani::gui
