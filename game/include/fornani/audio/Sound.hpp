
#pragma once

#include <SFML/Graphics.hpp>
#include <capo/engine.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <ranges>
#include <string>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

enum class SoundType { gui, gameplay };

struct SoundProperties {
	SoundType type{};
	float volume{1.f};
	float pitch_offset{};
	float min_distance{};
	float max_distance{};
};

class Sound {
  public:
	explicit Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string_view label, SoundProperties const properties, sf::Vector2f position);
	void update(automa::ServiceProvider& svc);
	void play(bool repeat = false);
	void set_volume(float volume);
	void set_pitch(float pitch);
	void fade_out();
	void set_fading(bool fading) { m_fading = fading; }

	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto is_playing() const -> bool { return m_sound->is_playing(); }
	[[nodiscard]] auto is_running() const -> bool { return is_playing() && !delete_me; }

  private:
	float compute_attenuation(float distance, float min_distance, float max_distance);

	std::string m_label{};
	SoundProperties m_properties{};
	sf::Vector2f m_position{};
	util::Cooldown m_fade{};

	bool delete_me{};
	bool m_fading{};
	bool m_fading_out{};
	std::unique_ptr<capo::ISource> m_sound{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
