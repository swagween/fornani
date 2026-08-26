
#pragma once

#include <fornani/utils/Constants.hpp>
#include <algorithm>

namespace fornani {

constexpr auto default_base_volume_v = 0.5; // fornani defaults all sound producers to half volume. this can be adjusted in user settings.

class Volume {
  public:
	constexpr void set_dynamic(double const to) { m_dynamic_multiplier = std::clamp(to, 0.0, 1.0); }
	constexpr void set_base(double const to) { m_base_multiplier = std::clamp(to, 0.0, 1.0); }
	constexpr void adjust_dynamic(double const delta) { set_dynamic(m_dynamic_multiplier + delta); }
	constexpr void adjust_base(double const delta) { set_base(m_base_multiplier + delta); }

	constexpr [[nodiscard]] auto get() const -> double { return m_base_multiplier * m_dynamic_multiplier; }
	constexpr [[nodiscard]] auto get_base() const -> double { return m_base_multiplier; }
	constexpr [[nodiscard]] auto at_range() const -> bool { return m_base_multiplier < constants::tiny_value || m_base_multiplier > 1.0 - constants::tiny_value; }

  private:
	double m_base_multiplier{default_base_volume_v};
	double m_dynamic_multiplier{1.f};
};

} // namespace fornani
