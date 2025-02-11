#pragma once
#include "BitFlags.hpp"

namespace fornani::util {
enum class Flags { state, trigger };
class StateFlag {
	constexpr void set() {
		flags.set(Flags::trigger);
		flags.set(Flags::state);
	}
	constexpr void reset() {
		flags.reset(Flags::trigger);
		flags.reset(Flags::state);
	}
	[[nodiscard]] constexpr auto test() const -> bool { return flags.test(Flags::state); }
	[[nodiscard]] constexpr auto consume_trigger() const -> bool {
		auto const ret = flags.test(Flags::trigger);
		flags.reset(Flags::trigger);
		return ret;
	}

	BitFlags<Flags> flags{};
};

} // namespace fornani::util
