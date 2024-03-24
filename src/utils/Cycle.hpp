
#pragma once
#include <algorithm>

namespace util {

class Cycle {
  public:
	Cycle(int o) { order = o; }
	constexpr void update() { alternator = alternator == order - 1 ? 0 : alternator + 1; }
	constexpr bool is_cycled() const { return alternator == 0; }
	constexpr int get_alternator() const { return alternator; }

  private:
	int alternator{};
	int order{};
};

} // namespace util