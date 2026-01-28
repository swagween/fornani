
#include <fornani/audio/Sound.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::audio {

SoundProperties SoundProperties::from_json(dj::Json const& in) {
	SoundProperties p{};
	p.volume = in["volume"].as<float>();
	p.max_distance = in["max_distance"] ? in["max_distance"].as<float>() : default_max_distance_v;
	p.min_distance = in["min_distance"] ? in["min_distance"].as<float>() : default_min_distance_v;
	p.pitch_offset = in["pitch_offset"].as<float>();
	p.fade_in = in["fade_in"].as<int>();
	p.fade_out = in["fade_out"].as<int>();
	return p;
}

Sound::Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string_view label, SoundProperties properties) : m_properties{properties} {
	auto source = engine.create_source();
	if (!source) { NANI_LOG_ERROR(m_logger, "Failed to create sound source"); }
	if (!source->bind_to(&buffer)) { NANI_LOG_ERROR(m_logger, "Failed to bind source"); };
	m_sound = std::move(source);
	m_sound->set_pitch(1.f + random::random_range_float(-properties.pitch_offset, properties.pitch_offset));
	if (properties.fade_in > 0) { m_fade_in = util::Cooldown{properties.fade_in}; }
	if (properties.fade_out > 0) {}
}

void Sound::update(automa::ServiceProvider& svc, sf::Vector2f position) {
	auto& sound = m_sound;
	auto attenuation = compute_attenuation(position.length(), m_properties.min_distance, m_properties.max_distance);
	m_actual_volume = m_properties.volume * attenuation;
	sound->set_gain(m_actual_volume);
	if (m_fade_in) {
		m_sound->set_gain(m_actual_volume * m_fade_in->get_inverse_normalized());
		m_fade_in->update();
		if (m_fade_in->is_almost_complete()) { m_fade_in.reset(); }
	}
	if (m_fade_out) {
		m_sound->set_gain(m_actual_volume * m_fade_out->get_normalized());
		m_fade_out->update();
	}
}

void Sound::play(bool repeat) {
	m_sound->set_looping(repeat);
	if (m_fade_in) {
		m_fade_in->start();
		m_sound->set_gain(m_actual_volume * m_fade_in->get_inverse_normalized());
	} else {
		m_sound->set_gain(m_actual_volume);
	}
	m_sound->play();
}

void Sound::set_volume(float volume) { m_sound->set_gain(volume * m_actual_volume); }

void Sound::set_pitch(float pitch) { m_sound->set_pitch(pitch); }

void Sound::fade_out() {
	if (is_looping()) {
		if (m_properties.fade_out > 0) {
			m_fade_out = util::Cooldown{m_properties.fade_out};
			m_fade_out->start();
		} else {
			m_sound->stop();
		}
		m_sound->set_looping(false);
	}
}

float Sound::compute_attenuation(float distance, float min_distance, float max_distance) {
	if (distance <= min_distance) return 1.f;
	if (distance >= max_distance) return 0.f;
	float t = (distance - min_distance) / (max_distance - min_distance);
	return 1.f - t;
}

[[nodiscard]] auto Sound::is_running() const -> bool {
	if (m_fade_in) {
		if (m_fade_in->running()) { return true; }
	}
	if (m_fade_out) {
		if (m_fade_out->is_complete()) { return false; }
	}
	return m_sound->is_playing();
}

} // namespace fornani::audio
