#include "SoundEffect.hpp"
#include <iostream>

namespace audio {

namespace fs = std::filesystem;

SoundEffect::SoundEffect(fs::path file) {
	if (!buffer.loadFromFile(file.string())) { std::cout << "sound effect load failure\n"; }
	sound.setBuffer(buffer);
}

} // namespace audio
