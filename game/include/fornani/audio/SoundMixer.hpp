
#pragma once

#include <fornani/audio/Volume.hpp>

namespace fornani::audio {

class SoundMixer {
  public:
	Volume volume{1.0};

	[[nodiscard]] auto mix(double const input) const -> double { return volume.get() * input; }

  private:
};

} // namespace fornani::audio
