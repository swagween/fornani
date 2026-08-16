
#include <fornani/graphics/Colors.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/WorldTimer.hpp>

namespace fornani {

WorldTimer::WorldTimer(automa::ServiceProvider& svc) : m_text{svc.text.fonts.title.font}, m_flash{80}, m_ticker{&svc.ticker} {
	m_text.setFillColor(colors::ui_white);
	m_text.setCharacterSize(16);
}

void WorldTimer::restart() {
	m_flags.reset(WorldTimerFlags::running);
	m_flags.reset(WorldTimerFlags::paused);
	m_counter.cancel();
}

void WorldTimer::start() {
	m_flags.set(WorldTimerFlags::running);
	m_flags.reset(WorldTimerFlags::paused);
	m_counter.cancel();
}

void WorldTimer::pause() {
	if (!is_running()) { return; }
	m_flags.set(WorldTimerFlags::paused);
}

void WorldTimer::resume() {
	if (is_running() && is_paused()) { m_flags.reset(WorldTimerFlags::paused); }
}

void WorldTimer::update() {
	if (is_paused() || !is_running()) { return; }
	m_counter.update();
}

void WorldTimer::finish(automa::ServiceProvider& svc) {
	m_flags.reset(WorldTimerFlags::running);
	m_flash.start();
	svc.data.time_trial_registry.register_time(svc, m_course, m_player_tag, get_time());
}

void WorldTimer::render(sf::RenderWindow& win, sf::Vector2f position) {
	if (m_flash.running()) {
		m_flash.get() % 16 < 8 ? m_text.setFillColor(colors::goldenrod) : m_text.setFillColor(colors::red);
	} else {
		m_text.setFillColor(colors::ui_white);
	}
	m_flash.update();
	m_text.setPosition(position);
	m_text.setString(std::format("{:.3f}", get_time()));
	win.draw(m_text);
}

void WorldTimer::debug() {}

} // namespace fornani
