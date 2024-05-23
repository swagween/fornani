
#pragma once
#include <algorithm>

namespace util {

class Counter {
  public:
	constexpr void start() { incrementor = 0; }
	constexpr void update() { incrementor = std::clamp(++incrementor, 0, INT_MAX); }
	constexpr void set(const int value) { incrementor = value; }
	constexpr void cancel() { incrementor = -1; }
	[[nodiscard]] auto running() const -> bool { return incrementor != 0; }
	[[nodiscard]] auto get_count() const -> int { return incrementor; }

  private:
	int incrementor{};
};

} // namespace util