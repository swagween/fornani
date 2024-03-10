
#pragma once

#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"

namespace player {

constexpr static int request_time{16};
constexpr static int cooldown_time{16};

enum class JumpTrigger { just_jumped, jump_launched, jumpsquat, is_released };

enum class JumpState {
	can_jump,	   // true if the player is grounded (USED)
	jump_held,	   // to prevent deceleration being called after jumping
	jumpsquatting, // (USED)
	is_pressed,	   // true if jump released midair, reset upon landing (USED)
	jumping,	   // true if jumpsquat is over, false once player lands (USED)
	jump_began     // true if just jumped, but must be active for a cooldown period to avoid next-frame cancelling
};

class Jump {
  public:
	void update();
	void reset_triggers();
	void reset_all();

	void request_jump();
	bool requested() const;
	bool released() const;
	bool began() const;
	bool can_jump() const;
	bool jumping() const;
	bool just_jumped() const;
	bool held() const;
	bool jumpsquatting() const;
	bool jumpsquat_trigger() const;
	void start_jumpsquat();
	void stop_jumpsquatting();
	void reset_jumpsquat_trigger();
	void reset_just_jumped();
	void start();
	void reset();
	void execute();
	void prevent();
	int get_request() const;

	util::BitFlags<JumpTrigger> triggers{};
	util::BitFlags<JumpState> states{};

	private:
	util::Cooldown cooldown{};
	util::Cooldown request{};
};

} // namespace player
