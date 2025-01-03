
#pragma once

#include <SFML/Audio.hpp>

namespace audio {

class Sound {
  public:
	void play();
	void set_volume(float volume);
	void set_pitch(float pitch);
	void set_buffer(sf::SoundBuffer& buffer);
	[[nodiscard]] auto get_status() const->sf::Sound::Status { return sound.getStatus(); }

  private:
	sf::Sound sound{};
};

} // namespace fornani