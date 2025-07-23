
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/WorldTimer.hpp>

namespace fornani {

WorldTimer::WorldTimer(automa::ServiceProvider& svc) : m_text{svc.text.fonts.title}, m_flash{80} {
	m_text.setFillColor(colors::ui_white);
	m_text.setCharacterSize(16);
}

void WorldTimer::restart() {
	m_timer = util::Stopwatch{};
	m_flags.reset(WorldTimerFlags::running);
}

void WorldTimer::start() {
	m_timer.start();
	m_flags.set(WorldTimerFlags::running);
}

void WorldTimer::finish(automa::ServiceProvider& svc) {
	m_timer.stop();
	m_flags.reset(WorldTimerFlags::running);
	m_flash.start();
	svc.data.time_trial_registry.register_time(svc, m_course, m_player_tag, m_timer.get_time());
}

void WorldTimer::render(sf::RenderWindow& win, sf::Vector2f position) {
	is_running() ? m_text.setString(m_timer.get_readout()) : m_text.setString(m_timer.get_final());
	if (m_flash.running()) {
		m_flash.get() % 16 < 8 ? m_text.setFillColor(colors::goldenrod) : m_text.setFillColor(colors::red);
	} else {
		m_text.setFillColor(colors::ui_white);
	}
	m_flash.update();
	m_text.setPosition(position);
	win.draw(m_text);
}

} // namespace fornani
