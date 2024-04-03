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
enum class TransponderInput { skip, next, exit, down, up, left, right, select, skip_released };
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
	void uninspect();
	void stop_dashing();
	void decrement_requests();
	void reset_dash_count();
	void cancel_dash_request();
	void dash();
	void autonomous_walk();
	void stop_walking_autonomously();
	void set_shot(bool flag);
	void prevent_movement();
	void release_hook();
	void nullify_dash();

	std::optional<float> get_controller_state(ControllerInput key) const;

	[[nodiscard]] auto nothing_pressed() -> bool { return key_map[ControllerInput::move_x] == 0.f && key_map[ControllerInput::jump] == 0.f && key_map[ControllerInput::inspect] == 0.f; }
	[[nodiscard]] auto moving() -> bool { return key_map[ControllerInput::move_x] != 0.f; }
	[[nodiscard]] auto moving_left() -> bool { return key_map[ControllerInput::move_x] < 0.f; }
	[[nodiscard]] auto moving_right() -> bool { return key_map[ControllerInput::move_x] > 0.f; }
	[[nodiscard]] auto facing_left() const -> bool { return direction.lr == dir::LR::left; }
	[[nodiscard]] auto facing_right() const -> bool { return direction.lr == dir::LR::right; }
	[[nodiscard]] auto restricted() const -> bool { return flags.test(MovementState::restricted); }
	[[nodiscard]] auto grounded() const -> bool { return flags.test(MovementState::grounded); }
	[[nodiscard]] auto walking_autonomously() const -> bool { return flags.test(MovementState::walking_autonomously); }
	[[nodiscard]] auto dash_requested() const -> bool { return dash_request > -1; }
	[[nodiscard]] auto shot() -> bool { return key_map[ControllerInput::shoot] == 1.f; }
	[[nodiscard]] auto released_hook() -> bool {
		auto ret = hook_flags.test(Hook::hook_released);
		hook_flags.reset(Hook::hook_released);
		return ret;
	}
	[[nodiscard]] auto hook_held() const -> bool { return hook_flags.test(Hook::hook_held); }
	[[nodiscard]] auto inspecting() -> bool { return key_map[ControllerInput::inspect] == 1.f; }
	[[nodiscard]] auto dashing() -> bool { return key_map[ControllerInput::dash] != 0.f; }
	[[nodiscard]] auto can_dash() const -> bool { return dash_count == 0; }
	[[nodiscard]] auto transponder_skip() const -> bool { return transponder_flags.test(TransponderInput::skip); }
	[[nodiscard]] auto transponder_skip_released() const -> bool { return transponder_flags.test(TransponderInput::skip_released); }
	[[nodiscard]] auto transponder_next() const -> bool { return transponder_flags.test(TransponderInput::next); }
	[[nodiscard]] auto transponder_exit() const -> bool { return transponder_flags.test(TransponderInput::skip); }
	[[nodiscard]] auto transponder_up() const -> bool { return transponder_flags.test(TransponderInput::up); }
	[[nodiscard]] auto transponder_down() const -> bool { return transponder_flags.test(TransponderInput::down); }
	[[nodiscard]] auto transponder_left() const -> bool { return transponder_flags.test(TransponderInput::left); }
	[[nodiscard]] auto transponder_right() const -> bool { return transponder_flags.test(TransponderInput::right); }
	[[nodiscard]] auto transponder_select() const -> bool { return transponder_flags.test(TransponderInput::select); }

	[[nodiscard]] auto get_dash_request() const -> int { return dash_request; }
	[[nodiscard]] auto get_dash_count() const -> int { return dash_count; }

	[[nodiscard]] auto vertical_movement() -> float { return key_map[ControllerInput::move_y]; }
	[[nodiscard]] auto horizontal_movement() -> float { return key_map[ControllerInput::move_x]; }
	[[nodiscard]] auto arms_switch() -> float { return key_map[ControllerInput::arms_switch]; }
	[[nodiscard]] auto dash_value() -> float { return key_map[ControllerInput::dash]; }

	[[nodiscard]] auto get_jump() -> Jump& { return jump; }

	dir::Direction direction{};

  private:
	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{}; // unused
	Jump jump{};
	util::BitFlags<TransponderInput> transponder_flags{};
	util::BitFlags<Hook> hook_flags{};

	int dash_request{};
	int dash_count{};
};
} // namespace player
