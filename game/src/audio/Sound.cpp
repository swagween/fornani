
#include <fornani/audio/Sound.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::audio {

Sound::Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string_view label, SoundProperties const properties, sf::Vector2f position) : m_properties{properties}, m_position{position}, m_label{label} {
	m_fade.start();
	auto source = engine.create_source();
	if (!source) { NANI_LOG_ERROR(m_logger, "Failed to create sound source"); }
	if (!source->bind_to(&buffer)) { NANI_LOG_ERROR(m_logger, "Failed to bind source"); };
	m_sound = std::move(source);
	if (properties.type == SoundType::gameplay) { m_sound->set_spatialized(true); }

	m_properties.volume *= compute_attenuation(position.length(), 100.f, 1000.f);
}

void Sound::update(automa::ServiceProvider& /*svc*/) {
	auto& sound = m_sound;
	m_fading ? sound->set_gain(m_properties.volume * m_fade.get_inverse_normalized()) : sound->set_gain(m_properties.volume);
	m_fading_out ? m_fade.reverse() : m_fade.update();
	if (m_fading_out && m_fade.started()) { delete_me = true; }
	// if (sound->is_spatialized()) { sound->set_position(capo::Vec3f{m_position.x, m_position.y, 0.f}); }
}

void Sound::play(bool repeat) {
	m_sound->set_looping(repeat);
	m_sound->set_gain(m_properties.volume);
	m_sound->play();
	if (m_label == "jump_low") { NANI_LOG_DEBUG(m_logger, "Volume: {}", m_properties.volume); }
}

void Sound::set_volume(float volume) { m_sound->set_gain(volume * m_properties.volume); }

void Sound::set_pitch(float pitch) { m_sound->set_pitch(pitch); }

void Sound::fade_out() { m_fading_out = true; }

float Sound::compute_attenuation(float distance, float min_distance, float max_distance) {
	if (distance <= min_distance) return 1.f;
	if (distance >= max_distance) return 0.f;
	float t = (distance - min_distance) / (max_distance - min_distance);
	return 1.f - t * t;
}

} // namespace fornani::audio
