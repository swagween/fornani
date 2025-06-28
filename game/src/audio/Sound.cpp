
#include "fornani/audio/Sound.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::audio {

Sound::Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string const& label, int echo_count, int echo_rate) : m_label(label) {
	m_sounds.clear();
	echo.rate = echo_rate;
	echo.count = util::Cooldown{echo_count};
	echo.repeater.start(echo.rate);
	echo.count.start();
	for (auto i{0}; i <= echo_count; ++i) {
		auto source = engine.create_source();
		if (!source) { NANI_LOG_ERROR(m_logger, "Failed to create sound source"); }
		if (!source->bind_to(&buffer)) { NANI_LOG_ERROR(m_logger, "Failed to bind source"); };
		m_sounds.emplace_back(std::move(source));
	}
}

void Sound::update(automa::ServiceProvider& /*svc*/) {
	echo.repeater.update();
	if (!is_echoing()) { return; }
	if (echo.rate < 1) { return; }
	if (echo.repeater.get_cooldown() % echo.rate == 0) {
		echo.repeater.start(echo.rate);
		echo.count.update();
		m_sounds.at(echo.count.get_cooldown())->set_gain(native_volume * echo.count.get_cubic_normalized() * 0.2f);
		m_sounds.at(echo.count.get_cooldown())->play();
	}
}

void Sound::play(bool repeat) {
	auto& sound = m_sounds.back();
	sound->set_looping(repeat);
	sound->play();
}

void Sound::set_volume(float volume) {
	native_volume = volume;
	m_sounds.back()->set_gain(volume);
}

void Sound::set_pitch(float pitch) {
	for (auto& s : m_sounds) { s->set_pitch(pitch); }
}

} // namespace fornani::audio
