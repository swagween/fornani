
#pragma once

#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <juke/juke.hpp>
#include <optional>

namespace fornani::audio {

enum class MusicPlayerState { on, off };
enum class MusicPlayerFlags { filtering };
constexpr auto lo_pass_v = 100.f;
constexpr auto hi_pass_v = 500.f;
constexpr auto default_filter_fade_speed_v = 128;

class MusicPlayer {
  public:
	explicit MusicPlayer(capo::IEngine& audio_engine);
	void quick_play(ResourceFinder const& finder, std::string_view song_name);
	void load(ResourceFinder const& finder, std::string_view song_name);
	void load(std::string_view path);
	void play_song_by_id(ResourceFinder const& finder, int id);
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
	void filter_fade_in(float const hi = hi_pass_v, float const lo = lo_pass_v, int speed = default_filter_fade_speed_v);
	void filter_fade_out();

	[[nodiscard]] auto get_volume() const -> float { return m_jukebox.get_gain(); }
	[[nodiscard]] auto get_fade() const -> util::Cooldown { return m_filter.fade; }
	[[nodiscard]] auto is_on() const -> bool { return m_state == MusicPlayerState::on; }
	[[nodiscard]] auto is_off() const -> bool { return m_state == MusicPlayerState::off; }

	[[nodiscard]] auto is_playing() const -> bool { return m_jukebox.is_playing(); }
	[[nodiscard]] auto is_stopped() const -> bool { return m_jukebox.is_stopped(); }

  private:
	MusicPlayerState m_state{};
	util::BitFlags<MusicPlayerFlags> m_flags{};
	juke::Jukebox m_jukebox;
	juke::Jukebox m_ringtone;
	std::string m_current_song{};
	struct {
		float lo{juke::sample_rate_v};
		float hi{0.f};
		float hi_target{};
		float lo_target{};
		util::Cooldown fade{};
	} m_filter{};

	std::unordered_map<int, std::string> m_name_from_id{};

	io::Logger m_logger{"Audio"};
};

} // namespace fornani::audio
