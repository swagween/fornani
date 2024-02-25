#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <chrono>
#include <optional>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Direction.hpp"


namespace controllers {

	using Clock = std::chrono::steady_clock;
	using Time = std::chrono::duration<float>;

	constexpr static int jump_time{16};

enum class ControllerInput { move_x, jump, shoot, arms_switch, inspect };
enum class MovementState { restricted, grounded };
enum class Jump {		// true if jump is pressed and permanently false once released, until player touches the ground again (USED)
	trigger,		// true for one frame if jump is pressed and the player is grounded (UNUSED)
	can_jump,		// true if the player is grounded (USED)
	just_jumped,	// used for updating animation (USED)
	jump_launched,	// successful jump, set player's y acceleration! (USED)
	jump_held,		// to prevent deceleration being called after jumping
	jumpsquatting,	// (USED)
	jumpsquat_trigger, //(USED)
	is_pressed,		// true if the jump button is pressed, false if not. independent of player's state.
	is_released,	// true if jump released midair, reset upon landing (USED)
	jumping		// true if jumpsquat is over, false once player lands (USED)
};

class PlayerController {

  public:
	PlayerController();

	void update();
	void jump();
	void prevent_jump();
	void stop();
	void ground();
	void unground();
	void restrict();
	void unrestrict();

	void start_jumping();
	void reset_jump();
	void decrement_jump();

	void start_jumpsquat();
	void stop_jumpsquatting();
	void reset_jumpsquat_trigger();
	void reset_just_jumped();

	void set_shot(bool flag);
	float arms_switch();

	std::optional<float> get_controller_state(ControllerInput key) const;

	bool moving();
	bool moving_left();
	bool moving_right();
	bool facing_left() const;
	bool facing_right() const;
	bool restricted() const;
	bool grounded() const;

	bool jump_requested() const;
	bool jump_released() const;
	bool can_jump() const;
	bool jumping() const;
	bool just_jumped() const;
	bool jump_held() const;

	bool shot();

	bool inspecting();

	bool jumpsquatting() const;
	bool jumpsquat_trigger() const;

	int get_jump_request() const;

	
	dir::Direction direction{};

  private:
	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{}; //unused
	util::BitFlags<Jump> jump_flags{};

	int jump_request{};

	// fixed animation time step variables
	Time dt{0.001f};
	Clock::time_point current_time = Clock::now();
	Time accumulator{0.0f};

};
} // namespace controllers
