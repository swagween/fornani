
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

#include <cstdint>

namespace fornani::player {

constexpr static int perfect_walljump{4};
constexpr static int walljump_request_time{12};

enum class WallslideTrigger : std::uint8_t { walljump_requested };
enum class WallslideState : std::uint8_t { wallsliding, walljump };

class Wallslide {
  public:
	void start();
	void end();
	void update();
	void reset_triggers();
	void reset_all();
	void request_walljump();

	[[nodiscard]] auto is_wallsliding() const -> bool { return states.test(WallslideState::wallsliding); }
	[[nodiscard]] auto walljump_requested() const -> bool { return triggers.test(WallslideTrigger::walljump_requested); }

	util::BitFlags<WallslideTrigger> triggers{};
	util::BitFlags<WallslideState> states{};

	private:
	util::Cooldown walljump_request{};
};

} // namespace fornani::player
