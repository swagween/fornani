#pragma once
#include <cassert>
#include <algorithm>

namespace util {

class Circuit {
  public:
	constexpr Circuit(int order, int selection = 0) : order(order), selection(selection) { assert(order > 0 && selection >= 0); }
	constexpr void modulate(int amount) { selection = (selection + order + amount) % order; }
	constexpr void zero() { selection = 0; }
	constexpr void set(int to_selection) { selection = to_selection % order; }
	constexpr void set_order(int to_order) {
		order = to_order;
		selection = std::min(selection, order - 1);
	}
	[[nodiscard]] constexpr auto get_order() const -> int { return order; }
	[[nodiscard]] constexpr auto get() const -> int { return selection; }
	[[nodiscard]] constexpr auto cycled() const -> bool { return selection == 0; }

  private:
	int order{};
	int selection{};
};

} // namespace util