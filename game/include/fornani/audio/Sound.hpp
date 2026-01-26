
#pragma once

#include <SFML/Graphics.hpp>
#include <capo/engine.hpp>
#include <djson/json.hpp>
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
	float min_distance{128.f};
	float max_distance{1024.f};
	int fade_in{0};
	int fade_out{0};
	static SoundProperties from_json(dj::Json const& in);
};

class Sound {
  public:
	explicit Sound(capo::IEngine& engine, capo::Buffer const& buffer, std::string_view label, SoundProperties properties);
	void update(automa::ServiceProvider& svc, sf::Vector2f position);
	void play(bool repeat = false);
	void set_volume(float volume);
	void set_pitch(float pitch);
	void fade_out();

	[[nodiscard]] auto is_playing() const -> bool { return m_sound->is_playing(); }
	[[nodiscard]] auto is_looping() const -> bool { return m_sound->is_looping(); }
	[[nodiscard]] auto is_running() const -> bool;

  private:
	float compute_attenuation(float distance, float min_distance, float max_distance);

	SoundProperties m_properties{};
	float m_actual_volume{};

	std::optional<util::Cooldown> m_fade_in{};
	std::optional<util::Cooldown> m_fade_out{};

	std::unique_ptr<capo::ISource> m_sound{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
