#pragma once
#include <SFML/Graphics.hpp>
#include <chrono>
#include <optional>
#include <unordered_map>
#include "../../utils/BitFlags.hpp"
#include "../../utils/Direction.hpp"
#include "Jump.hpp"

namespace player {

constexpr static int dash_time{32};

enum class ControllerInput { move_x, jump, shoot, arms_switch, inspect, dash, move_y };
enum class TransponderInput { skip, next, exit };
enum class MovementState { restricted, grounded, walking_autonomously };

enum class Hook { hook_released, hook_held };

class PlayerController {

  public:
	PlayerController();

	void update();
	void clean();
	void stop();
	void ground();
	void unground();
	void restrict();
	void unrestrict();

	void stop_dashing();

	void decrement_requests();

	void reset_dash_count();
	void cancel_dash_request();
	void dash();

	void autonomous_walk();
	void stop_walking_autonomously();

	void set_shot(bool flag);
	float arms_switch();

	void prevent_movement();
	void release_hook();

	std::optional<float> get_controller_state(ControllerInput key) const;

	bool nothing_pressed();

	bool moving();
	bool moving_left();
	bool moving_right();
	bool facing_left() const;
	bool facing_right() const;
	bool restricted() const;
	bool grounded() const;
	bool walking_autonomously() const;

	float vertical_movement();
	float horizontal_movement();

	bool dash_requested() const;

	bool shot();
	bool released_hook();
	bool hook_held() const;

	bool inspecting();
	bool dashing();
	bool can_dash();

	bool transponder_skip() const;
	bool transponder_next() const;
	bool transponder_exit() const;

	int get_dash_request() const;
	int get_dash_count() const;
	float dash_value();

	dir::Direction direction{};
	player::Jump& get_jump();

  private:
	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{}; // unused
	player::Jump jump{};
	util::BitFlags<TransponderInput> transponder_flags{};
	util::BitFlags<Hook> hook_flags{};

	int dash_request{};
	int dash_count{};
};
} // namespace controllers
