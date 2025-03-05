
#include "fornani/gui/Dashboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/gizmos/ClockGizmo.hpp"
#include "fornani/gui/gizmos/MapGizmo.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::gui {

Dashboard::Dashboard(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f dimensions)
	: m_physical{.dimensions{dimensions}}, m_sprite{svc.assets.get_texture("pioneer_dashboard")}, m_constituents{.top_left_frontplate{{{}, {153, 124}}, {}},
																												 .top_right_frontplate{{{153, 0}, {64, 127}}, {290.f, 0.f}},
																												 .arsenal_frontplate{{{26, 127}, {184, 137}}, {52, 218}},
																												 .top_left_slot{{{217, 0}, {153, 124}}, {}},
																												 .top_right_slot{{{370, 0}, {64, 127}}, {290.f, 0.f}},
																												 .arsenal_slot{{{253, 127}, {184, 137}}, {52, 218}},
																												 .motherboard{{{434, 0}, {222, 212}}, {14.f, 68.f}}},
	  m_paths{.map{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "dashboard_minimap", 32, util::InterpolationType::quadratic}} {
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
	m_paths.map.set_section("start");

	// populate dashboard depending on the player's inventory
	auto const& items = svc.data.get_player_items();
	for (auto& i : items.array_view()) {
		if (i["id"].as<int>() == 16) { m_gizmos.push_back(std::make_unique<MapGizmo>(svc, map, player)); }
	}
	auto clock_placement{sf::Vector2f{84.f, 142.f}};
	m_gizmos.push_back(std::make_unique<ClockGizmo>(svc, map, clock_placement)); // have to stick this in the for loop once we have a clock item
	m_sprite.setScale(util::constants::f_scale_vec);
}

void Dashboard::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	auto& controller = svc.controller_map;
	m_paths.map.update();
	for (auto& gizmo : m_gizmos) { gizmo->update(svc, player, map, m_physical.physics.position + m_paths.map.get_position()); }
	m_physical.physics.simple_update();
}

void Dashboard::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2f cam) {

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
		if (button.state == GizmoButtonState::hovered && m_gizmos.size() > 0) { m_current_gizmo = ccm::ext::clamp(ctr, 0, static_cast<int>(m_gizmos.size() - 1)); }
		button.box.setPosition(svc.window->f_center_screen() + pos);
		button.state == GizmoButtonState::hovered ? button.box.setOutlineColor(svc.styles.colors.bright_orange) : button.box.setOutlineColor(svc.styles.colors.dark_fucshia);
		// win.draw(button.box);
		++ctr;
	}

	// dashboard constituents
	auto render_position{-m_physical.physics.position + cam};
	m_constituents.motherboard.render(win, m_sprite, render_position, {});
	m_constituents.top_left_slot.render(win, m_sprite, render_position - m_paths.map.get_position(), {});
	m_constituents.top_right_slot.render(win, m_sprite, render_position - m_paths.map.get_position() - m_paths.map.get_dimensions(), {});
	m_constituents.arsenal_slot.render(win, m_sprite, render_position, {});
	for (auto& gizmo : m_gizmos) { gizmo->render(svc, win, player, cam, false); }
	m_constituents.top_left_frontplate.render(win, m_sprite, render_position - m_paths.map.get_position(), {});
	m_constituents.top_right_frontplate.render(win, m_sprite, render_position - m_paths.map.get_position() - m_paths.map.get_dimensions(), {});
	m_constituents.arsenal_frontplate.render(win, m_sprite, render_position, {});
	for (auto& gizmo : m_gizmos) { gizmo->render(svc, win, player, cam, true); }
}

bool Dashboard::handle_inputs(config::ControllerMap& controller, audio::Soundboard& soundboard) {
	if (m_gizmos.empty() || m_gizmos.size() <= m_current_gizmo) { return false; }
	if (!m_gizmos.at(m_current_gizmo)->handle_inputs(controller, soundboard)) {
		m_paths.map.set_section("close");
		return false;
	}
	return true;
}

void Dashboard::set_position(sf::Vector2f to_position, bool force) {
	if (force) {
		m_physical.physics.position = to_position;
	} else {
		m_physical.steering.seek(m_physical.physics, to_position, 0.002f);
	}
}

void Dashboard::set_selection(sf::Vector2i to_selection) { m_selected_position = to_selection; }

void Dashboard::select_gizmo() {
	if (m_gizmos.empty() || m_gizmos.size() <= m_current_gizmo) { return; }
	m_gizmos.at(m_current_gizmo)->select();
	if (m_gizmos.at(m_current_gizmo)->get_label() == "Minimap") { m_paths.map.set_section("open"); }
}

void Dashboard::hover(sf::Vector2i direction) {}

} // namespace fornani::gui
