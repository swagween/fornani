
#pragma once
#include <algorithm>

namespace util {

class Counter {
  public:
	constexpr void start() { incrementor = 0; }
	constexpr void update() { ++incrementor; }
	constexpr void cancel() { incrementor = 0; }
	[[nodiscard]] auto running() const -> bool { return incrementor != 0; }
	[[nodiscard]] auto get_count() const -> int { return incrementor; }

  private:
	int incrementor{};
};

} // namespace util