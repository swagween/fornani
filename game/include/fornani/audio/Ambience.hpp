
#pragma once

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "MusicPlayer.hpp"
#include <thread>

namespace automa {
struct ServiceProvider;
}

namespace audio {

class Ambience {
  public:
	void load(data::ResourceFinder& finder, std::string_view source);
	void play();
	void set_balance(float balance);
	struct {
		MusicPlayer open{};
		MusicPlayer closed{};
	} tracks{};
	float volume_multiplier = 0.05f;
};

} // namespace audio
