#pragma once

#include <SFML/Audio.hpp>
#include <filesystem>

namespace audio {

namespace fs = std::filesystem;

class SoundEffect {
  public:
	SoundEffect(fs::path file);
	SoundEffect(SoundEffect const&) = delete;
	SoundEffect& operator=(SoundEffect const& other) = delete;
	void play() { sound.play(); }
	sf::Sound sound;
  private:
	sf::SoundBuffer buffer;
};

} // namespace audio
