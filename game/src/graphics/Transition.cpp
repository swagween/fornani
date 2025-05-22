
#include "fornani/graphics/Transition.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::graphics {

Transition::Transition(sf::Vector2f screen_dim, int duration, sf::Color color) : m_cooldown{duration}, m_color{color}, m_box{screen_dim}, m_alpha{255} {}

void Transition::update(player::Player& player) {
	auto tt = static_cast<float>(m_cooldown.get_native_time());
	if ((m_fade_in || m_fade_out) && !player.controller.walking_autonomously()) { player.controller.restrict_movement(); }
	if (m_fade_out) {
		auto timer = (tt - static_cast<float>(m_cooldown.get_cooldown())) / tt;
		m_alpha = static_cast<std::uint8_t>(std::lerp(0, 255, timer));
		if (m_cooldown.is_complete()) {
			m_fade_out = false;
			m_done = true;
		}
	} else if (m_fade_in) {
		auto timer = static_cast<float>(m_cooldown.get_cooldown()) / tt;
		m_alpha = static_cast<std::uint8_t>(std::lerp(0, 255, timer));
		if (m_cooldown.is_complete()) { m_fade_in = false; }
	}
	if (m_done) {
		m_alpha = 255;
		m_hang_time.update();
	} else if (!(m_fade_in || m_fade_out)) {
		m_alpha = 0;
		player.controller.unrestrict();
	}
	m_cooldown.update();
}

void Transition::render(sf::RenderWindow& win) {
	if (m_fade_out || m_fade_in || m_done) {
		m_color.a = m_alpha;
		m_box.setFillColor(m_color);
		win.draw(m_box);
	}
}

void Transition::start() {
	if (!m_fade_out) { m_cooldown.start(); }
	m_fade_in = false;
	m_fade_out = true;
}

void Transition::end() {
	if (!m_fade_in) {
		m_cooldown.start();
		m_hang_time.cancel();
	}
	m_done = false;
	m_fade_in = true;
	m_fade_out = false;
}

} // namespace fornani::graphics
