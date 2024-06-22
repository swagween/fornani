#pragma once
#include <utility>

namespace util {

class Shipment {
  public:
	constexpr void set(int const value) {
		if (value == residue) { return; }
		pulse = value;
		residue = value;
	}
	[[nodiscard]] auto get_residue() const -> int { return residue; }
	[[nodiscard]] auto consume_pulse() -> int { return std::exchange(pulse, 0); }

  private:
	int pulse{};
	int residue{};
};

} // namespace util