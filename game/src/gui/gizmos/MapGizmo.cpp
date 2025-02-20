
#include "fornani/gui/gizmos/MapGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapGizmo::MapGizmo(automa::ServiceProvider& svc, world::Map& map) : Gizmo("Minimap"), m_minimap(svc), m_sprite{svc.assets.t_map_gizmo}, m_map_screen(svc, 48, 1) {
	m_position = sf::Vector2f{0.f, svc.constants.f_screen_dimensions.y};
	for (auto& id : svc.data.discovered_rooms) { m_minimap.bake(svc, map, id, id == svc.current_room); }
	m_minimap.center();
	m_sprite.setScale(svc.constants.texture_scale);
}

void MapGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::update(svc, player, map);
	m_minimap.update(svc, map, player);
}

void MapGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Gizmo::render(svc, win, cam);
	m_minimap.render(svc, win, m_position + cam);
}

bool MapGizmo::handle_inputs(config::ControllerMap& controller) {
	if (controller.digital_action_status(config::DigitalAction::menu_up).held) { m_minimap.move({0.f, -1.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_down).held) { m_minimap.move({0.f, 1.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_left).held) { m_minimap.move({-1.f, 0.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_right).held) { m_minimap.move({1.f, 0.f}); }
	if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) { m_minimap.toggle_scale(); }
	return Gizmo::handle_inputs(controller);
}

} // namespace fornani::gui
