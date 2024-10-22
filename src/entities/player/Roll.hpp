
#pragma once

#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../utils/Counter.hpp"

namespace player {

enum class RollFlags { rolling, can_roll };

class Roll {
  public:
	void update();
	void request();
	void reset();

	[[nodiscard]] auto is_rolling() const -> bool { return roll_window.running(); }

  private:
	util::Cooldown roll_window{12};
	util::BitFlags<RollFlags> flags{};
};

} // namespace player
