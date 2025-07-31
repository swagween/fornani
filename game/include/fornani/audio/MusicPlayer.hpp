
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
	void quick_play(ResourceFinder const& finder, std::string_view song_name);
	void load(ResourceFinder const& finder, std::string_view song_name);
	void load(std::string_view path);
	void play_once();
	void play_looped();
	void update();
	void pause();
	void stop();
	void resume();
	void fade_out(std::chrono::duration<float> duration);
	void fade_in(std::chrono::duration<float> duration);
	void turn_on();
	void turn_off();
	void set_volume(float vol);
	void adjust_volume(float delta);

	[[nodiscard]] auto get_volume() const -> float { return m_jukebox.get_gain(); }
	[[nodiscard]] auto is_on() const -> bool { return m_state == MusicPlayerState::on; }
	[[nodiscard]] auto is_off() const -> bool { return m_state == MusicPlayerState::off; }

	[[nodiscard]] auto is_playing() const -> bool { return m_jukebox.is_playing(); }
	[[nodiscard]] auto is_stopped() const -> bool { return m_jukebox.is_stopped(); }

  private:
	MusicPlayerState m_state{};
	juke::Jukebox m_jukebox;
	juke::Jukebox m_ringtone;
	std::string m_current_song{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
