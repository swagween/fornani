
#pragma once
#include <limits>

#include <ccmath/ext/clamp.hpp>

namespace fornani::util {

class Counter {
  public:
	constexpr void start() { incrementor = 0; }
	constexpr void update(int const amount = 1) { incrementor = ccm::ext::clamp(incrementor + amount, 0, std::numeric_limits<int>::max()); }
	constexpr void set(int const value) { incrementor = value; }
	constexpr void cancel() { incrementor = -1; }
	[[nodiscard]] auto running() const -> bool { return incrementor != 0; }
	[[nodiscard]] auto canceled() const -> bool { return incrementor == -1; }
	[[nodiscard]] auto get_count() const -> int { return incrementor; }

  private:
	int incrementor{};
};

} // namespace fornani::util
