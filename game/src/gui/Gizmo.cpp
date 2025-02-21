
#include "fornani/gui/Gizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

void Gizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {}

void Gizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {}

bool Gizmo::handle_inputs(config::ControllerMap& controller) {
	if (controller.digital_action_status(config::DigitalAction::menu_cancel).triggered) { return false; }
	return true;
}

} // namespace fornani::gui
