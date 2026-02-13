
#pragma once

#include <ccmath/math/basic/min.hpp>
#include <fornani/io/Logger.hpp>
#include <cassert>

namespace fornani::util {

class Circuit {
  public:
	explicit Circuit(int const order, int const selection = 0) : m_order(order), m_selection(selection) {
		if (debug) { NANI_LOG_DEBUG(m_logger, "Ctor Order: {}", order); }
		assert(order > 0 && selection >= 0);
	}
	void modulate(int const amount) {
		if (debug) { NANI_LOG_DEBUG(m_logger, "Order before modulation: {}", m_order); }
		m_selection = (m_selection + m_order + amount) % m_order;
		if (debug) { NANI_LOG_DEBUG(m_logger, "Selection after modulation: {}", m_selection); }
		if (debug) { NANI_LOG_DEBUG(m_logger, "Order after modulation: {}", m_order); }
	}
	void zero() { m_selection = 0; }
	void set(int const to_selection) { m_selection = to_selection % m_order; }
	void set_order(int const to_order) {
		if (debug) { NANI_LOG_DEBUG(m_logger, "Order before setting: {}", m_order); }
		m_order = to_order;
		m_selection = ccm::min(m_selection, m_order - 1);
		if (debug) { NANI_LOG_DEBUG(m_logger, "Order after setting: {}", m_order); }
	}
	[[nodiscard]] auto get_order() const -> int { return m_order; }
	[[nodiscard]] auto get_normalized() const -> float { return static_cast<float>(m_selection) / static_cast<float>(m_order); }
	[[nodiscard]] auto get() const -> int { return m_selection; }
	[[nodiscard]] auto cycled() const -> bool { return m_selection == 0; }
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

	bool debug{};

  private:
	int m_order{};
	int m_selection{};

	io::Logger m_logger{"util"};
};

} // namespace fornani::util
