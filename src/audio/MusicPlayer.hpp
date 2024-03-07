
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"
#include "../setup/ResourceFinder.hpp"
#include <thread>

namespace audio {

enum class Song { clay_statue };
enum class SongState { playing, paused, off };

class MusicPlayer {
  public:
	void load(std::string song_name);
	void play_once();
	void play_looped();
	void update();
	void pause();
	void stop();
	void fade_out();
	void fade_in();
	void turn_off();
	void turn_on();

	data::ResourceFinder finder{};

  private:
	util::BitFlags<Song> song_flags{};
	util::BitFlags<SongState> state_flags{};
	int current_loop{};

	sf::Music song_first{};
	sf::Music song_loop{};
	sf::SoundSource::Status status{};

	sf::Time start_time{};
	sf::Time end_time{};
	sf::Time current_time{};
	sf::Clock music_clock{};

};

} // namespace audio
