
#include "fornani/gui/gizmos/ClockGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

ClockGizmo::ClockGizmo(automa::ServiceProvider& svc, world::Map& map) : Gizmo("Minimap"), m_sprites{.clock{sf::Sprite{svc.assets.get_texture("clock_gizmo")}}, .hand{sf::Sprite{svc.assets.get_texture("clock_hand")}}} {
	m_physics.position = sf::Vector2f{0.f, 0.f};
	m_sprites.clock.setScale(svc.constants.texture_scale);
	m_sprites.hand.setScale(svc.constants.texture_scale);
}

void ClockGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) { Gizmo::update(svc, player, map); }

void ClockGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Gizmo::render(svc, win, cam);
	win.draw(m_sprites.clock);
}

bool ClockGizmo::handle_inputs(config::ControllerMap& controller) { return Gizmo::handle_inputs(controller); }

} // namespace fornani::gui
