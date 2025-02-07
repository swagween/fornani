#include "fornani/audio/Sound.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::audio {

Sound::Sound(const sf::SoundBuffer& buffer, int echo_count, int echo_rate) {
	sounds.clear();
	echo.rate = echo_rate;
	echo.count = util::Cooldown{echo_count};
	echo.repeater.start(echo.rate);
	echo.count.start();
	for (auto i{0}; i <= echo_count; ++i) { sounds.emplace_back(buffer); }
}

void Sound::update(automa::ServiceProvider& /*svc*/) {
	echo.repeater.update();
	if (!is_echoing()) { return; }
	if (echo.rate < 1) { return; }
	if (echo.repeater.get_cooldown() % echo.rate == 0) {
		echo.repeater.start(echo.rate);
		echo.count.update();
		sounds.at(echo.count.get_cooldown()).setVolume(native_volume * echo.count.get_cubic_normalized() * 0.2f);
		sounds.at(echo.count.get_cooldown()).play();
	}
}

void Sound::play() { 
	auto& sound = sounds.back();
	sound.play();
	//sound.setEffectProcessor(sf::SoundSource::EffectProcessor)
}

void Sound::set_volume(float volume) {
	native_volume = volume;
	sounds.back().setVolume(volume);
}

void Sound::set_pitch(float pitch) {
	for (auto& s : sounds) { s.setPitch(pitch); }
}

} // namespace fornani
