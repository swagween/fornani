#pragma once
#include <cassert>

namespace util {

class Circuit {
  public:
	constexpr Circuit(int order, int selection = 0) : order(order), selection(selection) { assert(order > 0 && selection >= 0); }
	constexpr void modulate(int amount) { selection = (selection + order + amount) % order; }
	constexpr [[nodiscard]] auto get_order() const -> int { return order; }
	constexpr [[nodiscard]] auto get() const -> int { return selection; }

  private:
	int order{};
	int selection{};
};

} // namespace util