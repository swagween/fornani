
#pragma once

#include <SFML/Graphics.hpp>
#include "MusicPlayer.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

class Ambience {
  public:
	explicit Ambience(capo::IEngine& audio_engine);
	void load(ResourceFinder& finder, std::string_view source);
	void play();
	void set_balance(float balance);
	void set_volume(float const vol) { m_in_game_multiplier = vol; }
	struct {
		MusicPlayer open;
		MusicPlayer closed;
	} tracks;

  private:
	float m_volume_multiplier;
	float m_in_game_multiplier;
};

} // namespace fornani::audio
