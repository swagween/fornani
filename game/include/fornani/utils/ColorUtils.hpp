#pragma once

#include <SFML/Graphics.hpp>

#include <ccmath/math/misc/lerp.hpp>
#include <random>
#include "fornani/utils/Cooldown.hpp"

namespace fornani::util {

constexpr static int default_fader_time{64};

struct ColorUtils {
	static sf::Color fade_in(sf::Color const color, int rate = m_fader.get_native_time()) {
		if (m_fader.is_almost_complete()) { return color; }
		if (!m_fader.running()) { m_fader.start(rate); }
		auto ret{color};
		ret.a = ccm::lerp(0u, 255u, m_fader.get_inverse_normalized());
		m_fader.update();
		return ret;
	}

	static void reset() { m_fader = Cooldown{default_fader_time}; }

  private:
	inline static Cooldown m_fader{default_fader_time};
};

} // namespace fornani::util
