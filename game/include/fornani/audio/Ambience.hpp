
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/audio/Balance.hpp>
#include <fornani/audio/MusicPlayer.hpp>
#include <fornani/audio/Volume.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

class Ambience {
  public:
	explicit Ambience(capo::IEngine& audio_engine);
	void load(ResourceFinder& finder, std::string_view source);
	void play();
	void set_balance(double balance);

	Volume volume{};
	struct {
		MusicPlayer open;
		MusicPlayer closed;
	} tracks;

  private:
	std::string current_track{};
};

} // namespace fornani::audio
