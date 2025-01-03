#include "Sound.hpp"

namespace audio {

void Sound::play() { sound.play(); }

void Sound::set_volume(float volume) { sound.setVolume(volume); }

void Sound::set_pitch(float pitch) { sound.setPitch(pitch); }

void Sound::set_buffer(sf::SoundBuffer& buffer) { sound.setBuffer(buffer); }

} // namespace fornani