
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"
#include "../setup/ResourceFinder.hpp"

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class SongState { playing, paused, on, looping };
enum class MusicPlayerState { on };

class MusicPlayer {
  public:
	void load(std::string_view song_name);
	void simple_load(std::string_view source);
	void play_once(float vol = 100.f);
	void play_looped(float vol = 100.f);
	void update();
	void pause();
	void stop();
	void fade_out();
	void fade_in();
	void switch_on();
	void switch_off();
	void turn_off();
	void turn_on();
	void set_volume(float vol);
	[[nodiscard]] auto get_volume() const -> float { return song_first.getStatus() == sf::SoundSource::Status::Playing ? song_first.getVolume() : song_loop.getStatus() == sf::SoundSource::Status::Playing ? song_loop.getVolume() : 0.f; }
	[[nodiscard]] auto global_off() const -> bool { return !flags.player.test(MusicPlayerState::on); }
	[[nodiscard]] auto switched_off() const -> bool { return !flags.state.test(SongState::on); }

	[[nodiscard]] auto playing() const -> bool { return song_first.getStatus() == sf::SoundSource::Status::Playing || song_loop.getStatus() == sf::SoundSource::Status::Playing; }

	data::ResourceFinder finder{};

	struct {
		float native{};
		float actual{};
		float multiplier{1.0f};
	} volume{};

  private:
	struct {
		util::BitFlags<SongState> state{};
		util::BitFlags<MusicPlayerState> player{};
	} flags{};

	int current_loop{};

	sf::Music song_first{};
	sf::Music song_loop{};
	sf::SoundSource::Status status{};
	std::int64_t last_dt{};

	sf::Time start_time{};
	sf::Time end_time{};
	sf::Time current_time{};
	sf::Clock music_clock{};
	sf::Clock music_tick{};

	std::string label{};
};

} // namespace audio
