
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"
#include "../setup/ResourceFinder.hpp"
#include <thread>

namespace automa {
struct ServiceProvider;
}

namespace audio {

enum class Song { clay_statue };
enum class SongState { playing, paused, on };
enum class MusicPlayerState { on };

class MusicPlayer {
  public:
	void load(std::string_view song_name);
	void play_once(int vol = 100);
	void play_looped(int vol = 100);
	void update();
	void pause();
	void stop();
	void fade_out();
	void fade_in();
	void switch_on();
	void switch_off();
	void turn_off();
	void turn_on();
	void set_volume(int vol);
	[[nodiscard]] auto get_volume() const -> int { return song_first.getStatus() == sf::SoundSource::Status::Playing ? song_first.getVolume() : song_loop.getStatus() == sf::SoundSource::Status::Playing ? song_loop.getVolume() : 0; }
	[[nodiscard]] auto global_off() const -> bool { return !flags.player.test(MusicPlayerState::on); }
	[[nodiscard]] auto playing() const -> bool { return song_first.getStatus() == sf::SoundSource::Status::Playing || song_loop.getStatus() == sf::SoundSource::Status::Playing; }

	data::ResourceFinder finder{};

	struct {
		int native{};
		int actual{};
		float multiplier{1.0f};
	} volume{};

  private:
	struct {
		util::BitFlags<Song> song{};
		util::BitFlags<SongState> state{};
		util::BitFlags<MusicPlayerState> player{};
	} flags{};

	int current_loop{};

	sf::Music song_first{};
	sf::Music song_loop{};
	sf::SoundSource::Status status{};

	sf::Time start_time{};
	sf::Time end_time{};
	sf::Time current_time{};
	sf::Clock music_clock{};

	std::string_view label{};

};

} // namespace audio
