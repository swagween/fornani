
#include "fornani/gui/gizmos/MapGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapGizmo::MapGizmo(automa::ServiceProvider& svc, world::Map& map)
	: Gizmo("Minimap"), m_minimap{std::make_unique<MiniMap>(svc)}, m_sprite{svc.assets.get_texture("map_gizmo")}, m_map_screen(svc, svc.assets.get_texture("map_screen"), {45, 45}, {1, 1}),
	  m_map_shadow(svc, svc.assets.get_texture("map_shadow"), {45, 45}, {1, 1}), m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "minimap", 32, util::InterpolationType::quadratic},
	  m_constituents{.gizmo{.top_left{.lookup{{0, 0}, {67, 55}}},
							.top_right{.lookup{{67, 0}, {63, 55}}, .position{134.f, 0.f}},
							.bottom_left{.lookup{{13, 55}, {54, 54}}, .position{26.f, 110.f}},
							.bottom_right{.lookup{{67, 55}, {61, 55}}, .position{134.f, 110.f}}}} {
	m_physics.position = sf::Vector2f{0.f, svc.constants.f_screen_dimensions.y};
	for (auto& id : svc.data.discovered_rooms) { m_minimap->bake(svc, map, id, id == svc.current_room); }
	m_minimap->center();
	m_sprite.setScale(svc.constants.texture_scale);
	m_path.set_section("close");
	m_placement = {380.f, -22.f};
}

void MapGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		m_path.set_section("open");
		m_switched = false;
	} else if (m_switched) {
		m_path.set_section("close");
		m_switched = false;
	}
	m_path.update();
	m_minimap->update(svc, map, player);
	m_minimap->set_port_position(m_path.get_position() + m_placement - m_map_screen.get_f_corner_dimensions());
	m_minimap->set_port_dimensions(m_map_screen.get_bounds());
	m_map_screen.set_position(m_path.get_position() + m_placement);
	m_map_screen.set_dimensions(m_path.get_dimensions());
	m_map_shadow.set_position(m_path.get_position() + m_placement);
	m_map_shadow.set_dimensions(m_path.get_dimensions());
	if (m_flags.toggled) {
		m_minimap->toggle_scale();
		m_flags.toggled = false;
	}
}

void MapGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Gizmo::render(svc, win, cam);
	auto render_position{-m_placement + cam};
	m_map_screen.render(win, cam);
	m_minimap->render(svc, win, cam);
	m_map_shadow.render(win, cam);
	m_constituents.gizmo.top_left.position = m_path.get_position();
	m_constituents.gizmo.top_left.render(win, m_sprite, render_position, sf::Vector2f{66.f, 54.f});
	m_constituents.gizmo.top_right.position = m_path.get_position() + sf::Vector2f{m_path.get_dimensions().x, 0.f};
	m_constituents.gizmo.top_right.render(win, m_sprite, render_position, sf::Vector2f{1.f, 54.f});
	m_constituents.gizmo.bottom_left.position = m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y};
	m_constituents.gizmo.bottom_left.render(win, m_sprite, render_position, sf::Vector2f{53.f, 1.f});
	m_constituents.gizmo.bottom_right.position = m_path.get_position() + m_path.get_dimensions();
	m_constituents.gizmo.bottom_right.render(win, m_sprite, render_position, sf::Vector2f{1.f, 1.f});
}

bool MapGizmo::handle_inputs(config::ControllerMap& controller) {
	if (controller.digital_action_status(config::DigitalAction::menu_up).held) { m_minimap->move({0.f, -1.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_down).held) { m_minimap->move({0.f, 1.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_left).held) { m_minimap->move({-1.f, 0.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_right).held) { m_minimap->move({1.f, 0.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) { m_flags.toggled = true; }
	return Gizmo::handle_inputs(controller);
}

} // namespace fornani::gui
