
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/WorldTimer.hpp>

namespace fornani {

WorldTimer::WorldTimer(automa::ServiceProvider& svc) : m_text{svc.text.fonts.title}, m_flash{80}, m_dt{&svc.ticker.dt_scalar} {
	m_text.setFillColor(colors::ui_white);
	m_text.setCharacterSize(16);
}

void WorldTimer::restart() {
	m_timer = util::Stopwatch{};
	m_flags.reset(WorldTimerFlags::running);
	m_flags.reset(WorldTimerFlags::paused);
}

void WorldTimer::start() {
	m_timer.start();
	m_flags.set(WorldTimerFlags::running);
	m_flags.reset(WorldTimerFlags::paused);
}

void WorldTimer::pause() {
	if (!is_running()) { return; }
	m_timer.stop();
	m_flags.set(WorldTimerFlags::paused);
}

void WorldTimer::resume() {
	if (is_running() && is_paused()) {
		m_timer.resume();
		m_flags.reset(WorldTimerFlags::paused);
	}
}

void WorldTimer::finish(automa::ServiceProvider& svc) {
	m_timer.stop();
	m_flags.reset(WorldTimerFlags::running);
	m_flash.start();
	svc.data.time_trial_registry.register_time(svc, m_course, m_player_tag, m_timer.get_time() * *m_dt);
}

void WorldTimer::render(sf::RenderWindow& win, sf::Vector2f position) {
	is_running() && !is_paused() ? m_text.setString(m_timer.get_readout(*m_dt)) : m_text.setString(m_timer.get_final(*m_dt));
	if (m_flash.running()) {
		m_flash.get() % 16 < 8 ? m_text.setFillColor(colors::goldenrod) : m_text.setFillColor(colors::red);
	} else {
		m_text.setFillColor(colors::ui_white);
	}
	m_flash.update();
	m_text.setPosition(position);
	win.draw(m_text);
}

void WorldTimer::debug() { m_timer.print_time("world timer"); }

} // namespace fornani
