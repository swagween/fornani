
#pragma once

namespace fornani::util {

class Cycle {
  public:
	explicit Cycle(int const o) { order = o; }
	constexpr void update() { alternator = alternator == order - 1 ? 0 : alternator + 1; }
	constexpr bool is_cycled() const { return alternator == 0; }
	constexpr int get_alternator() const { return alternator; }
	[[nodiscard]] constexpr auto get_normalized() const -> float { return static_cast<float>(alternator) / static_cast<float>(order); }

	void set_order(int const o) { order = o; }
	void reset() { alternator = 0; }
	void set(int const to) { alternator = to; }

  private:
	int alternator{};
	int order{};
};

} // namespace fornani::util
