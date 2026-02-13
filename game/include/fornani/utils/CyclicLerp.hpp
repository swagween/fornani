
#pragma once

#include <ccmath/math/misc/lerp.hpp>
#include <algorithm>
#include <cmath>

namespace fornani {

class CyclicLerp {
  public:
	CyclicLerp(float range = 1.f) : m_range(range) {}

	void trigger(int cycles, bool forward, float duration) {
		m_start = m_position;

		float distance = m_range * static_cast<float>(cycles);
		m_end = m_start + (forward ? distance : -distance);

		m_duration = duration;
		m_time = 0.f;
		m_active = true;
	}

	void update(float dt) {
		if (!m_active) return;

		m_time += dt;

		float t = std::clamp(m_time / m_duration, 0.f, 1.f);

		// Strong ease-out curve
		// Fast start, slower finish
		t = eas_out_cubic(t);

		m_position = ccm::lerp(m_start, m_end, t);

		if (m_time >= m_duration) {
			m_position = m_end;
			m_active = false;
		}
	}

	[[nodiscard]] auto get_velocity() const -> float { return 1.f - m_position; }
	[[nodiscard]] auto get_range() const -> float { return m_range; }
	[[nodiscard]] auto raw() const -> float { return m_position; }
	[[nodiscard]] auto get() const -> float {
		float wrapped = std::fmod(m_position, m_range);
		if (wrapped < 0.f) wrapped += m_range;
		return wrapped;
	}

	bool is_active() const { return m_active; }

  private:
	static float eas_out_cubic(float t) {
		float inv = 1.f - t;
		return 1.f - inv * inv * inv;
	}
	float m_range;

	float m_position{};
	float m_start{};
	float m_end{};

	float m_time{0.f};
	float m_duration{0.25f};

	bool m_active{false};
};

} // namespace fornani
