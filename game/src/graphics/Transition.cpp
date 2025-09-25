
#include "fornani/graphics/Transition.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::graphics {

Transition::Transition(sf::Vector2f screen_dim, int duration, sf::Color color) : m_cooldown{duration}, m_color{color}, m_box{screen_dim}, m_alpha{255} {}

void Transition::update(player::Player& player) {
	if ((is(TransitionState::black) || is(TransitionState::fading_to_black) || is(TransitionState::fading_out)) && !player.controller.walking_autonomously()) {
		player.controller.restrict_movement();
	} else {
		player.controller.unrestrict();
	}

	// set alpha
	switch (m_state) {
	case TransitionState::inactive: m_alpha = 0; break;
	case TransitionState::fading_to_black: m_alpha = static_cast<std::uint8_t>(std::lerp(0, 255, m_cooldown.get_inverse_normalized())); break;
	case TransitionState::black: m_alpha = 255; break;
	case TransitionState::fading_out: m_alpha = static_cast<std::uint8_t>(std::lerp(0, 255, m_cooldown.get_normalized())); break;
	}

	if (is(TransitionState::black)) { m_hang_time.update(); }
	if (m_cooldown.is_almost_complete()) { m_state = is(TransitionState::fading_to_black) ? TransitionState::black : TransitionState::inactive; }
	m_cooldown.update();
}

void Transition::render(sf::RenderWindow& win) {
	if (is(TransitionState::inactive)) { return; }
	m_color.a = m_alpha;
	m_box.setFillColor(m_color);
	win.draw(m_box);
}

auto Transition::as_string() const -> std::string {
	switch (m_state) {
	case TransitionState::inactive: return "inactive"; break;
	case TransitionState::fading_to_black: return "fading_to_black"; break;
	case TransitionState::black: return "black"; break;
	case TransitionState::fading_out: return "fading_out"; break;
	}
	return "?";
}

void Transition::start() {
	m_cooldown.start();
	m_state = TransitionState::fading_to_black;
	NANI_LOG_DEBUG(m_logger, "Started");
}

void Transition::end() {
	m_cooldown.start();
	m_state = TransitionState::fading_out;
	m_hang_time.cancel();
	NANI_LOG_DEBUG(m_logger, "Ended");
}

} // namespace fornani::graphics
