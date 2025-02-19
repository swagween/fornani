
#pragma once

#include <SFML/Graphics.hpp>
#include "MusicPlayer.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {

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
