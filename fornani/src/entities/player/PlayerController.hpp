#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <chrono>
#include "../../utils/BitFlags.hpp"

namespace controllers {

	using Clock = std::chrono::steady_clock;
	using Time = std::chrono::duration<float>;

	constexpr static int jump_time{12};

enum class Movement { move_x, jump };
enum class MovementState { restricted, grounded };
enum class Jump {
	hold,		 // true if jump is pressed and permanently false once released, until player touches the ground again
	trigger,	 // true for one frame if jump is pressed and the player is grounded
	can_jump,	 // true if the player is grounded
	just_jumped, // used for updating animation
	is_pressed,	 // true if the jump button is pressed, false if not. independent of player's state.
	is_released, // true if jump released midair, reset upon landing
	jumping,	 // true if jumpsquat is over, falce once player lands
};
enum class Direction { left, right };

class PlayerController {

  public:
	PlayerController();

	void update();
	void jump();
	void sustain_jump();
	void prevent_jump();
	void stop();
	void ground();
	void unground();

	void start_jumping();
	void set_jump_hold();
	void reset_jump();
	void reset_just_jumped();
	void reset_jump_flags();
	void decrement_jump();

	float& get_controller_state(Movement key);

	bool moving();
	bool moving_left();
	bool moving_right();
	bool facing_left() const;
	bool facing_right() const;
	bool restricted() const;

	bool jump_requested() const;
	bool just_jumped();
	bool jump_triggered() const;
	bool jump_released() const;


  private:
	std::unordered_map<Movement, float> key_map{};
	util::BitFlags<MovementState> flags{}; //unused
	util::BitFlags<Jump> jump_flags{};
	Direction direction{};

	int jump_request{};

	// fixed animation time step variables
	Time dt{0.001f};
	Clock::time_point current_time = Clock::now();
	Time accumulator{0.0f};

};
} // namespace controllers
