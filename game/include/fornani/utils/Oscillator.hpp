
#pragma once

#include <ccmath/math/basic.hpp>

namespace fornani {

class Oscillator {
  public:
	void trigger(float strength = 1.f) { m_velocity = strength; }
	void update(float dt);

	[[nodiscard]] auto get() const -> float { return m_position; }
	[[nodiscard]] auto get_zero_centered() const -> float { return ccm::abs(m_position); }

	void set_stiffness(float k) { m_stiffness = k; }
	void set_damping(float d) { m_damping = d; }

  private:
	float m_position{0.f};
	float m_velocity{0.f};
	float m_target{0.f};

	float m_stiffness{120.f};
	float m_damping{14.f};
};

} // namespace fornani
