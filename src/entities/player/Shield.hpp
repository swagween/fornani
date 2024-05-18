#pragma once

#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"

namespace player {

enum class ShieldTrigger { shield_up, shield_down };
enum class ShieldState { shielding };

class Shield {
  public:
	void start();
	void end();
	void update();
	void reset_triggers();
	void reset_all();

	[[nodiscard]] auto is_shielding() const -> bool { return states.test(ShieldState::shielding); }
	[[nodiscard]] auto shield_up() const -> bool { return triggers.test(ShieldTrigger::shield_up); }
	[[nodiscard]] auto shield_down() const -> bool { return triggers.test(ShieldTrigger::shield_down); }

	util::BitFlags<ShieldTrigger> triggers{};
	util::BitFlags<ShieldState> states{};

	private:
		struct {
		  int time{2800};
		} stats{};
	util::Cooldown timer{};
};

} // namespace player
