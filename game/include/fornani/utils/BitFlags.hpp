
#pragma once

#include <bitset>

namespace fornani::util {

template <typename Enum, std::size_t Size = 64u>
struct BitFlags {
	std::bitset<Size> bits{};

	bool test(Enum e) const { return bits[static_cast<std::size_t>(e)]; }
	void set(Enum e) { bits.set(static_cast<std::size_t>(e)); }
	void reset(Enum e) { bits.reset(static_cast<std::size_t>(e)); }
	void toggle(Enum e) { test(e) ? reset(e) : set(e); }
	bool consume(Enum e) {
		bool const ret = bits[static_cast<std::size_t>(e)];
		reset(e);
		return ret;
	}
	bool any(BitFlags const& other) const { return (bits & other.bits).any(); }

	bool operator==(BitFlags const&) const = default;
};

} // namespace fornani::util
