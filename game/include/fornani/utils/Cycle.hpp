
#pragma once

namespace fornani::util {

class Cycle {
  public:
	explicit Cycle(int const o) { order = o; }
	constexpr void update() { alternator = alternator == order - 1 ? 0 : alternator + 1; }
	constexpr bool is_cycled() const { return alternator == 0; }
	constexpr int get_alternator() const { return alternator; }
	void set_order(int const o) { order = o; }

  private:
	int alternator{};
	int order{};
};

} // namespace fornani::util