
#include "fornani/audio/Sound.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::audio {

Sound::Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string const& label, int echo_count, int echo_rate, float volume, int fade) : m_label(label), m_volume{volume}, m_fade{fade} {
	m_sounds.clear();
	m_fade.start();
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
	auto& sound = m_sounds.back();
	m_fading ? sound->set_gain(m_volume * m_fade.get_inverse_normalized()) : sound->set_gain(m_volume);
	m_fading_out ? m_fade.reverse() : m_fade.update();
	if (m_fading_out && m_fade.started()) { delete_me = true; }

	echo.repeater.update();
	if (!is_echoing()) { return; }
	if (echo.rate < 1) { return; }
	if (echo.repeater.get() % echo.rate == 0) {
		echo.repeater.start(echo.rate);
		echo.count.update();
		m_sounds.at(echo.count.get())->set_gain(m_volume * echo.count.get_cubic_normalized() * 0.2f);
		m_sounds.at(echo.count.get())->play();
	}
}

void Sound::play(bool repeat) {
	auto& sound = m_sounds.back();
	sound->set_looping(repeat);
	sound->set_gain(m_volume);
	sound->play();
}

void Sound::set_volume(float volume) { m_sounds.back()->set_gain(volume * m_volume); }

void Sound::set_pitch(float pitch) {
	for (auto& s : m_sounds) { s->set_pitch(pitch); }
}

void Sound::fade_out() { m_fading_out = true; }

} // namespace fornani::audio
