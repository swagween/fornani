
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/audio/Balance.hpp>
#include <fornani/audio/MusicPlayer.hpp>
#include <fornani/audio/SoundMixer.hpp>
#include <fornani/audio/Volume.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

class SoundMixer;

class Ambience {
  public:
	explicit Ambience(capo::IEngine& audio_engine, SoundMixer& mixer);
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

	SoundMixer* m_mixer;
};

} // namespace fornani::audio
