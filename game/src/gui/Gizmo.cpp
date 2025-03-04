
#include "fornani/gui/Gizmo.hpp"

#include "fornani/audio/Soundboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/setup/ControllerMap.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

void Gizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {}

void Gizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {}

bool Gizmo::handle_inputs(config::ControllerMap& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
		deselect();
		return false;
	}
	return true;
}

void Gizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	svc.soundboard.flags.pioneer.set(audio::Pioneer::select);
	m_switched = false;
}

void Gizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	svc.soundboard.flags.pioneer.set(audio::Pioneer::back);
	m_switched = false;
}

void Gizmo::select() {
	m_state = GizmoState::selected;
	m_switched = true;
}

void Gizmo::deselect() {
	m_state = GizmoState::hovered;
	m_switched = true;
}

void Constituent::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) const {
	sprite.setOrigin(origin);
	sprite.setPosition(position - cam);
	sprite.setTextureRect(lookup);
	win.draw(sprite);
}

} // namespace fornani::gui
