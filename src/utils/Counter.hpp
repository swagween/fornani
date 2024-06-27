
#pragma once
#include <algorithm>
#include <limits>

namespace util {

class Counter {
  public:
	constexpr void start() { incrementor = 0; }
	constexpr void update() { incrementor = std::clamp(incrementor + 1, 0, std::numeric_limits<int>::max()); }
	constexpr void set(const int value) { incrementor = value; }
	constexpr void cancel() { incrementor = -1; }
	[[nodiscard]] auto running() const -> bool { return incrementor != 0; }
	[[nodiscard]] auto canceled() const -> bool { return incrementor == -1; }
	[[nodiscard]] auto get_count() const -> int { return incrementor; }

  private:
	int incrementor{};
};

} // namespace util