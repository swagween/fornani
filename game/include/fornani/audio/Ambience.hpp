
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/audio/Balance.hpp>
#include <fornani/audio/MusicPlayer.hpp>

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
	void set_volume(float const vol) { m_volume_multiplier = vol; }
	void adjust_volume(float delta) { set_volume(get_volume() + delta); };
	struct {
		MusicPlayer open;
		MusicPlayer closed;
	} tracks;

	[[nodiscard]] auto get_volume() const -> float { return m_volume_multiplier; }

  private:
	std::string current_track{};
	float m_volume_multiplier;
};

} // namespace fornani::audio
