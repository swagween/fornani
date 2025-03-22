
#pragma once

#include <ccmath/math/basic.hpp>
#include <cassert>

namespace fornani::util {

class Circuit {
  public:
	constexpr explicit Circuit(int const order, int const selection = 0) : order(order), selection(selection) { assert(order > 0 && selection >= 0); }
	constexpr void modulate(int const amount) { selection = (selection + order + amount) % order; }
	constexpr void zero() { selection = 0; }
	constexpr void set(int const to_selection) { selection = to_selection % order; }
	constexpr void set_order(int const to_order) {
		order = to_order;
		selection = ccm::min(selection, order - 1);
	}
	[[nodiscard]] constexpr auto get_order() const -> int { return order; }
	[[nodiscard]] constexpr auto get() const -> int { return selection; }
	[[nodiscard]] constexpr auto cycled() const -> bool { return selection == 0; }
	template <typename T>
	T as() const {
		return static_cast<T>(selection);
	}

  private:
	int order{};
	int selection{};
};

} // namespace fornani::util
