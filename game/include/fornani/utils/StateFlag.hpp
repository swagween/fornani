#pragma once
#include "BitFlags.hpp"

namespace util {
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
		auto ret = flags.test(Flags::trigger);
		flags.reset(Flags::trigger);
		return ret;
	}
  private:
	BitFlags<Flags> flags{};
};

} // namespace util
