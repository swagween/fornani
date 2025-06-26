
#pragma once

#include <juke/juke.hpp>
#include <optional>
#include "fornani/io/Logger.hpp"
#include "fornani/setup/ResourceFinder.hpp"

namespace fornani::audio {

enum class MusicPlayerState : std::uint8_t { on, off };

class MusicPlayer {
  public:
	explicit MusicPlayer(capo::IEngine& audio_engine);
	void load(data::ResourceFinder const& finder, std::string_view song_name);
	void load(std::string_view path);
	void play_once();
	void play_looped();
	void update();
	void pause();
	void stop();
	void fade_out();
	void fade_in();
	void turn_on();
	void turn_off();
	void set_volume(float vol);
	void set_volume_multiplier(float to);

	[[nodiscard]] auto get_volume() const -> float { return 0.f; }
	[[nodiscard]] auto is_on() const -> bool { return m_state == MusicPlayerState::on; }
	[[nodiscard]] auto is_off() const -> bool { return m_state == MusicPlayerState::off; }

	[[nodiscard]] auto is_playing() const -> bool { return m_jukebox.is_playing(); }
	[[nodiscard]] auto is_stopped() const -> bool { return m_jukebox.is_stopped(); }

	[[nodiscard]] auto get_volume_multiplier() const -> float { return m_volume_multiplier; }

  private:
	MusicPlayerState m_state{};
	juke::Jukebox m_jukebox;

	float m_volume_multiplier{0.5f};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
