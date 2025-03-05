
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Direction.hpp"

namespace fornani::player {

enum class RollFlags : std::uint8_t { rolling, can_roll };

class Roll {
  public:
	void update();
	void request();
	void reset();
	void roll();
	void break_out();

	[[nodiscard]] auto is_valid() const -> bool { return roll_window.running(); }
	[[nodiscard]] auto rolling() const -> bool { return flags.test(RollFlags::rolling); }

	dir::Direction direction{};

  private:
	util::Cooldown roll_window{38};
	util::BitFlags<RollFlags> flags{};
};

} // namespace player
