
#pragma once

#include "../../utils/BitFlags.hpp"
#include "../../utils/Cooldown.hpp"
#include "../../utils/Counter.hpp"

namespace player {

constexpr static int request_time{16};
constexpr static int cooldown_time{40};

enum class JumpTrigger { just_jumped, jump_launched, jumpsquat, is_released };

enum class JumpState {
	can_jump,	   // true if the player is grounded (USED)
	jump_held,	   // to prevent deceleration being called after jumping
	jumpsquatting, // (USED)
	is_pressed,	   // true if jump released midair, reset upon landing (USED)
	jumping,	   // true if jumpsquat is over, false once player lands (USED)
	jump_began     // true if just jumped, but must be active for a cooldown period to avoid next-frame cancelling
};

enum class DoublejumpState { can_doublejump };

class Jump {
  public:
	void update();
	void reset_triggers();
	void reset_all();

	void start_coyote() { coyote_time.start(); }
	[[nodiscard]] auto coyote() const -> bool { return coyote_time.running(); }
	[[nodiscard]] auto can_doublejump() const -> bool { return jump_counter.get_count() == 0; }
	[[nodiscard]] auto is_doublejump() const -> bool { return jump_counter.get_count() >= 1; }
	[[nodiscard]] auto just_doublejumped() const -> bool { return jump_counter.get_count() == 1; }
	[[nodiscard]] auto get_coyote() const -> int { return coyote_time.get_cooldown(); }
	[[nodiscard]] auto get_count() const -> int { return jump_counter.get_count(); }

	void request_jump();
	void cancel();
	bool requested() const;
	bool launched() const;
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
	void reset_jumping();
	void start();
	void reset();
	void prevent();
	void doublejump();
	int get_request() const;
	int get_cooldown() const;

	util::BitFlags<JumpTrigger> triggers{};
	util::BitFlags<JumpState> states{};
	util::Counter jump_counter{};

	private:
	util::Cooldown cooldown{};
	util::Cooldown request{};
	util::Cooldown coyote_time{8};
};

} // namespace player
