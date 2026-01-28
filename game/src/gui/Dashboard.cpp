
#include "fornani/gui/Dashboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/gui/gizmos/ClockGizmo.hpp"
#include "fornani/gui/gizmos/InventoryGizmo.hpp"
#include "fornani/gui/gizmos/MapGizmo.hpp"
#include "fornani/gui/gizmos/WardrobeGizmo.hpp"
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
	  m_paths{.map{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "dashboard_minimap", 32, util::InterpolationType::quadratic}}, m_palette{"pioneer", svc.finder}, m_services{&svc} {
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

	auto clock_placement{sf::Vector2f{84.f, 142.f}};
	auto wardrobe_placement{sf::Vector2f{svc.window->f_screen_dimensions().x + 26.f, 0.f}};
	auto inventory_placement{sf::Vector2f{-svc.window->f_screen_dimensions().x + 366.f, 32.f}};

	// push gizmos in clockwise order so selection setting will work
	if (player.catalog.inventory.has_item("radar_device")) { m_gizmos.push_back(std::make_unique<MapGizmo>(svc, map, player)); }
	if (player.catalog.inventory.has_item("status_gizmo")) { m_gizmos.push_back(std::make_unique<WardrobeGizmo>(svc, map, wardrobe_placement)); }
	if (player.catalog.inventory.has_item("clock")) { m_gizmos.push_back(std::make_unique<ClockGizmo>(svc, map, clock_placement)); }
	m_gizmos.push_back(std::make_unique<InventoryGizmo>(svc, map, player, inventory_placement));

	m_sprite.setScale(constants::f_scale_vec);
}

void Dashboard::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	auto& controller = svc.controller_map;
	m_paths.map.update();
	m_light_up.update();
	m_light_shift.update();
	for (auto& gizmo : m_gizmos) {
		gizmo->update(svc, player, map, m_physical.physics.position + m_paths.map.get_position());
		if (m_current_port == DashboardPort::minimap && gizmo->get_label() == "Clock") { gizmo->select(); }
		m_current_port == gizmo->get_dashboard_port() ? gizmo->hover() : gizmo->neutralize();
	}
	m_physical.physics.simple_update();
}

void Dashboard::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, sf::Vector2f cam, LightShader& shader) {

	m_debug.box.setPosition(m_physical.physics.position - cam);
	auto pos{sf::Vector2f{}};
	auto ctr{0};
	auto dist{256.f};
	m_current_port = DashboardPort::invalid;
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
		if (button.state == GizmoButtonState::hovered) { m_current_port = static_cast<DashboardPort>(ccm::ext::clamp(ctr, 0, static_cast<int>(DashboardPort::invalid))); }
		button.box.setPosition(svc.window->f_center_screen() + pos);
		button.state == GizmoButtonState::hovered ? button.box.setOutlineColor(colors::bright_orange) : button.box.setOutlineColor(colors::dark_fucshia);

		++ctr;
	}

	auto max_dark = 2.f;
	auto darken_factor = m_light_shift.running() ? 1.f : max_dark;
	auto lighten_factor = m_light_up.running() ? 0.f : 0.f; // doesn't look good, but leaving it just in case

	// dashboard constituents
	auto render_position{-m_physical.physics.position + cam};
	is_home() ? shader.set_darken(lighten_factor) : shader.set_darken(darken_factor);
	m_constituents.motherboard.render(win, m_sprite, render_position, {}, shader, m_palette);
	m_constituents.top_left_slot.render(win, m_sprite, render_position - m_paths.map.get_position(), {}, shader, m_palette);
	m_constituents.top_right_slot.render(win, m_sprite, render_position - m_paths.map.get_position() - m_paths.map.get_dimensions(), {}, shader, m_palette);
	m_constituents.arsenal_slot.render(win, m_sprite, render_position, {}, shader, m_palette);
	for (auto& gizmo : m_gizmos) {
		shader.set_darken(max_dark);
		gizmo->render(svc, win, player, shader, m_palette, cam, false);
	}
	is_home() ? shader.set_darken(lighten_factor) : shader.set_darken(darken_factor);
	if (m_state == DashboardState::gizmo && m_current_port == DashboardPort::minimap) { shader.set_darken(lighten_factor); }
	m_constituents.top_left_frontplate.render(win, m_sprite, render_position - m_paths.map.get_position(), {}, shader, m_palette);
	m_constituents.top_right_frontplate.render(win, m_sprite, render_position - m_paths.map.get_position() - m_paths.map.get_dimensions(), {}, shader, m_palette);
	is_home() ? shader.set_darken(lighten_factor) : shader.set_darken(darken_factor);
	m_constituents.arsenal_frontplate.render(win, m_sprite, render_position, {}, shader, m_palette);
	for (auto& gizmo : m_gizmos) {
		shader.set_darken(max_dark);
		gizmo->render(svc, win, player, shader, m_palette, cam, true);
	}

	// for (auto& button : m_debug.buttons) { win.draw(button.box); }
}

bool Dashboard::handle_inputs(config::ControllerMap& controller, audio::Soundboard& soundboard) {
	for (auto& gizmo : m_gizmos) {
		if (m_current_port == gizmo->get_dashboard_port()) {
			if (!gizmo->handle_inputs(controller, soundboard)) {
				if (gizmo->get_dashboard_port() == DashboardPort::minimap) { m_paths.map.set_section("close"); } // only adjust dashboard art for map gizmo
				m_state = DashboardState::home;
				m_current_port = DashboardPort::invalid;
				m_selected_position = {};
				return false;
			}
		}
	}
	return true;
}

void Dashboard::set_position(sf::Vector2f to_position, bool force) {
	if (force) {
		m_physical.physics.position = to_position;
	} else {
		m_physical.steering.seek(m_physical.physics, to_position, 0.004f);
	}
}

void Dashboard::set_selection(sf::Vector2i to_selection) {
	auto same = m_selected_position == to_selection;
	m_selected_position = to_selection;
	auto switched = to_selection.x == 0 && to_selection.y == 0;
	if (is_home() && !switched && !same) {
		m_light_shift.start();
	} else if (is_hovering() && switched) {
		m_light_up.start();
	}
	m_state = switched ? DashboardState::home : DashboardState::hovering;
	switched ? m_services->soundboard.flags.pioneer.set(audio::Pioneer::unhover) : m_services->soundboard.flags.pioneer.set(audio::Pioneer::forward);
}

bool Dashboard::select_gizmo() {
	for (auto& gizmo : m_gizmos) {
		if (m_current_port == gizmo->get_dashboard_port()) {
			gizmo->select();
			m_state = DashboardState::gizmo;
			if (gizmo->get_label() == "Minimap") { m_paths.map.set_section("open"); } // uniquely, the minimap affects the Dashboard's constituents
			return true;
		}
	}
	return false;
}

void Dashboard::hover(sf::Vector2i direction) {}

void Dashboard::close() {
	for (auto& gizmo : m_gizmos) { gizmo->close(); }
}

} // namespace fornani::gui
