
#pragma once

#include <fornani/utils/BitFlags.hpp>

namespace fornani {

template <typename T>
class Flaggable {
  public:
	void set_flag(T const to_set, bool on = true) { on ? m_flags.set(to_set) : m_flags.reset(to_set); }
	void toggle_flag(T const to_toggle) { m_flags.test(to_toggle) ? m_flags.reset(to_toggle) : m_flags.set(to_toggle); }
	void reset_all() { m_flags = {}; }
	[[nodiscard]] auto has_flag_set(T const test) const -> bool { return m_flags.test(test); }
	[[nodiscard]] auto consume_flag(T const test) -> bool { return m_flags.consume(test); }

  private:
	util::BitFlags<T> m_flags{};
};

} // namespace fornani
