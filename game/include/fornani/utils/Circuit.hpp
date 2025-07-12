
#pragma once

#include <ccmath/math/basic.hpp>
#include <fornani/utils/Direction.hpp>
#include <cassert>

namespace fornani::util {

class Circuit {
  public:
	constexpr explicit Circuit(int const order, int const selection = 0) : m_order(order), m_selection(selection) { assert(order > 0 && selection >= 0); }
	constexpr void modulate(int const amount) { m_selection = (m_selection + m_order + amount) % m_order; }
	constexpr void zero() { m_selection = 0; }
	constexpr void set(int const to_selection) { m_selection = to_selection % m_order; }
	constexpr void set_order(int const to_order) {
		m_order = to_order;
		m_selection = ccm::min(m_selection, m_order - 1);
	}
	[[nodiscard]] constexpr auto get_order() const -> int { return m_selection; }
	[[nodiscard]] constexpr auto get() const -> int { return m_selection; }
	[[nodiscard]] constexpr auto cycled() const -> bool { return m_selection == 0; }
	template <typename T>
	T as() const {
		return static_cast<T>(m_selection);
	}
	template <typename T>
	T modulate_as(int amount) {
		modulate(amount);
		auto result = m_selection == m_order - 1 ? 0 : m_selection == 0 ? 1 : 2;
		if (m_order == 1) { result = amount < 0 ? 0 : 1; } // handle edge case of 1-order Circuit
		return static_cast<T>(result);
	}

  private:
	int m_order{};
	int m_selection{};
};

} // namespace fornani::util
