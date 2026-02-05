
#include "fornani/gui/Gizmo.hpp"

#include "fornani/audio/Soundboard.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/systems/InputSystem.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

void Gizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) { m_light_shift.update(); }

void Gizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_hovered() || is_selected()) {
		m_light_shift.running() ? shader.set_darken(0.f) : shader.set_darken(0.f);
	} else if (m_light_shift.running()) {
		shader.set_darken(1.f);
	}
}

bool Gizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.digital(input::DigitalAction::menu_back).triggered) {
		deselect();
		return false;
	}
	return true;
}

void Gizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	svc.soundboard.flags.pioneer.set(audio::Pioneer::select);
	select();
	m_switched = false;
}

void Gizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	if (m_switched) { svc.soundboard.flags.pioneer.set(audio::Pioneer::back); }
	deselect();
	m_switched = false;
}

void Gizmo::hover() {
	if (is_neutral()) {
		m_state = GizmoState::hovered;
		m_light_shift.start();
	}
}

void Gizmo::select() {
	m_state = GizmoState::selected;
	m_switched = true;
}

void Gizmo::deselect() {
	m_state = GizmoState::hovered;
	neutralize();
	m_switched = true;
}

void Gizmo::close() {
	m_state = GizmoState::closed;
	m_exit_trigger = true;
}

void Gizmo::neutralize() {
	if (is_hovered()) {
		m_state = GizmoState::neutral;
		m_light_shift.start();
	}
}

void Constituent::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin, LightShader& shader, Palette& palette) const {
	auto previous_origin = sprite.getOrigin();
	sprite.setOrigin(origin);
	sprite.setPosition(position - cam);
	sprite.setTextureRect(lookup);
	shader.submit(win, palette, sprite);
	sprite.setOrigin(previous_origin);
}

void Constituent::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) const {
	auto previous_origin = sprite.getOrigin();
	sprite.setOrigin(origin);
	sprite.setPosition(position - cam);
	sprite.setTextureRect(lookup);
	win.draw(sprite);
	sprite.setOrigin(previous_origin);
}

void FreeConstituent::update() {
	constituent.position = physics.position;
	physics.simple_update();
}

void Gizmo::report() {
	NANI_LOG_INFO(m_logger, "Report for Gizmo {}:", m_label);
	switch (m_state) {
	case GizmoState::neutral: NANI_LOG_INFO(m_logger, "	> neutral", m_label); break;
	case GizmoState::closed: NANI_LOG_INFO(m_logger, "	> closed", m_label); break;
	case GizmoState::hovered: NANI_LOG_INFO(m_logger, "	> hovered", m_label); break;
	case GizmoState::selected: NANI_LOG_INFO(m_logger, "	> selected", m_label); break;
	}
}

} // namespace fornani::gui
