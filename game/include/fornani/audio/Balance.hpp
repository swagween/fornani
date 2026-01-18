
#pragma once

#include <algorithm>
#include <cmath>

namespace fornani::audio {

enum class BalanceSource { scene, world, instant, obscure };
enum class BalanceTarget { music, ambience };

class SoundBalance {
  public:
	void set_target(float const to) { m_target = std::clamp(to, 0.f, 1.f); }
	void update(float const dt, float speed = 2.f, float recovery = 0.01f) {
		auto t = std::clamp(speed * dt, 0.0f, 1.0f);
		m_actual = std::lerp(m_actual, m_target, t);
		m_target = std::clamp(m_target + recovery, 0.f, 1.f);
	}
	[[nodiscard]] auto get() const -> float { return m_actual; }

  private:
	float m_actual{1.f};
	float m_target{1.f};
};

} // namespace fornani::audio
