
#pragma once

#include <SFML/Audio.hpp>
#include "fornani/utils/Cooldown.hpp"

namespace automa {
struct ServiceProvider;
}

namespace audio {

class Sound {
  public:
	Sound(const sf::SoundBuffer& buffer, int echo_count = 0, int echo_rate = 16);
	void update(automa::ServiceProvider& svc);
	void play();
	void set_volume(float volume);
	void set_pitch(float pitch);
	[[nodiscard]] auto get_status() const -> sf::Sound::Status {
		return std::ranges::find_if(sounds, [](auto& s) { return s.getStatus() == sf::Sound::Status::Playing; }) == std::ranges::end(sounds) ? sf::Sound::Status::Stopped : sf::Sound::Status::Playing;
	}
	[[nodiscard]] auto is_echoing() const -> bool { return echo.count.running(); }
	[[nodiscard]] auto is_playing() const -> bool { return get_status() == sf::Sound::Status::Playing; }
	[[nodiscard]] auto is_running() const -> bool { return is_playing() || is_echoing(); }

  private:
	float native_volume{100.f};
	std::vector<sf::Sound> sounds{};
	struct {
		util::Cooldown count{};
		util::Cooldown repeater{};
		int rate{};
	} echo{};
};

} // namespace audio
