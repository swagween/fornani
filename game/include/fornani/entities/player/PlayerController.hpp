#pragma once
#include <SFML/Graphics.hpp>
#include <chrono>
#include <optional>
#include <unordered_map>
#include <deque>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Direction.hpp"
#include "Jump.hpp"
#include "Wallslide.hpp"
#include "Shield.hpp"
#include "Slide.hpp"
#include "Roll.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {

constexpr static int dash_time{32};
constexpr static int quick_turn_sample_size{24};
constexpr static float backwards_dampen{0.5f};

enum class ControllerInput { move_x, jump, sprint, shield, shoot, arms_switch, inspect, dash, move_y, slide };
enum class TransponderInput { skip, next, exit, down, up, left, right, select, skip_released, hold_left, hold_right, hold_up, hold_down };
enum class MovementState { restricted, grounded, walking_autonomously, walljumping };
enum class HardState { no_move, has_arsenal };

enum class Hook { hook_released, hook_held };
enum class Sprint { released };

class PlayerController {

  public:
	PlayerController(automa::ServiceProvider& svc);

	void update(automa::ServiceProvider& svc);
	void clean();
	void stop();
	void ground();
	void unground();
	void restrict_movement();
	void unrestrict();
	void uninspect();
	void stop_dashing();
	void decrement_requests();
	void reset_dash_count();
	void cancel_dash_request();
	void dash();
	void walljump();
	void autonomous_walk();
	void stop_walking_autonomously();
	void set_shot(bool flag);
	void prevent_movement();
	void release_hook();
	void nullify_dash();
	void stop_walljumping();
	void set_arsenal(bool const has);

	std::optional<float> get_controller_state(ControllerInput key) const;

	[[nodiscard]] auto nothing_pressed() -> bool { return key_map[ControllerInput::move_x] == 0.f && key_map[ControllerInput::jump] == 0.f && key_map[ControllerInput::inspect] == 0.f; }
	[[nodiscard]] auto moving() -> bool { return key_map[ControllerInput::move_x] != 0.f; }
	[[nodiscard]] auto sprinting() -> bool { return key_map[ControllerInput::sprint] != 0.f; }
	[[nodiscard]] auto moving_left() -> bool { return key_map[ControllerInput::move_x] < 0.f; }
	[[nodiscard]] auto moving_right() -> bool { return key_map[ControllerInput::move_x] > 0.f; }
	[[nodiscard]] auto facing_left() const -> bool { return direction.lr == dir::LR::left; }
	[[nodiscard]] auto facing_right() const -> bool { return direction.lr == dir::LR::right; }
	[[nodiscard]] auto restricted() const -> bool { return flags.test(MovementState::restricted); }
	[[nodiscard]] auto grounded() const -> bool { return flags.test(MovementState::grounded); }
	[[nodiscard]] auto is_walljumping() const -> bool { return flags.test(MovementState::walljumping); }
	[[nodiscard]] auto walking_autonomously() const -> bool { return flags.test(MovementState::walking_autonomously); }
	[[nodiscard]] auto dash_requested() const -> bool { return dash_request > -1; }
	[[nodiscard]] auto shot() -> bool { return key_map[ControllerInput::shoot] == 1.f; }
	[[nodiscard]] auto sliding() -> bool { return key_map[ControllerInput::slide] != 0.f; }
	[[nodiscard]] auto released_hook() -> bool {
		auto ret = hook_flags.test(Hook::hook_released);
		hook_flags.reset(Hook::hook_released);
		return ret;
	}
	[[nodiscard]] auto has_arsenal() const -> bool { return hard_state.test(HardState::has_arsenal); }
	[[nodiscard]] auto hook_held() const -> bool { return hook_flags.test(Hook::hook_held); }
	[[nodiscard]] auto inspecting() -> bool { return key_map[ControllerInput::inspect] == 1.f; }
	[[nodiscard]] auto dashing() -> bool { return key_map[ControllerInput::dash] != 0.f; }
	[[nodiscard]] auto can_dash() const -> bool { return dash_count == 0; }
	[[nodiscard]] auto can_jump() const -> bool { return (flags.test(MovementState::grounded) || jump.coyote()) || jump.can_doublejump() || wallslide.is_wallsliding(); }
	[[nodiscard]] auto sprint_released() const -> bool { return sprint_flags.test(Sprint::released); }
	[[nodiscard]] auto transponder_skip() const -> bool { return transponder_flags.test(TransponderInput::skip); }
	[[nodiscard]] auto transponder_skip_released() const -> bool { return transponder_flags.test(TransponderInput::skip_released); }
	[[nodiscard]] auto transponder_next() const -> bool { return transponder_flags.test(TransponderInput::next); }
	[[nodiscard]] auto transponder_exit() const -> bool { return transponder_flags.test(TransponderInput::skip); }
	[[nodiscard]] auto transponder_up() const -> bool { return transponder_flags.test(TransponderInput::up); }
	[[nodiscard]] auto transponder_down() const -> bool { return transponder_flags.test(TransponderInput::down); }
	[[nodiscard]] auto transponder_left() const -> bool { return transponder_flags.test(TransponderInput::left); }
	[[nodiscard]] auto transponder_right() const -> bool { return transponder_flags.test(TransponderInput::right); }
	[[nodiscard]] auto transponder_select() const -> bool { return transponder_flags.test(TransponderInput::select); }
	[[nodiscard]] auto transponder_hold_up() const -> bool { return transponder_flags.test(TransponderInput::hold_up); }
	[[nodiscard]] auto transponder_hold_down() const -> bool { return transponder_flags.test(TransponderInput::hold_down); }
	[[nodiscard]] auto transponder_hold_left() const -> bool { return transponder_flags.test(TransponderInput::hold_left); }
	[[nodiscard]] auto transponder_hold_right() const -> bool { return transponder_flags.test(TransponderInput::hold_right); }

	[[nodiscard]] auto get_dash_request() const -> int { return dash_request; }
	[[nodiscard]] auto get_dash_count() const -> int { return dash_count; }

	[[nodiscard]] auto vertical_movement() -> float { return key_map[ControllerInput::move_y]; }
	[[nodiscard]] auto horizontal_movement() -> float { return key_map[ControllerInput::move_x]; }
	[[nodiscard]] auto sliding_movement() -> float { return key_map[ControllerInput::slide]; }
	[[nodiscard]] auto arms_switch() -> float { return key_map[ControllerInput::arms_switch]; }
	[[nodiscard]] auto dash_value() -> float { return key_map[ControllerInput::dash]; }
	[[nodiscard]] auto quick_turn() const -> bool {
		bool ret{};
		bool left{};
		bool right{};
		for (auto& state : horizontal_inputs) {
			if (state == -1.f) { left = true; }
			if (state == 1.f) { right = true; }
		}
		if (left && right) { ret = true; }
		return ret;
	}

	[[nodiscard]] auto get_jump() -> Jump& { return jump; }
	[[nodiscard]] auto get_wallslide() -> Wallslide& { return wallslide; }
	[[nodiscard]] auto get_shield() -> Shield& { return shield; }
	[[nodiscard]] auto get_slide() -> Slide& { return slide; }
	Roll roll{};

	dir::Direction direction{};

  private:
	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{}; // unused
	util::BitFlags<HardState> hard_state{}; // unused
	util::BitFlags<Sprint> sprint_flags{};
	util::BitFlags<TransponderInput> transponder_flags{};
	util::BitFlags<Hook> hook_flags{};
	
	Jump jump{};
	Wallslide wallslide{};
	Shield shield;
	Slide slide{};

	int dash_request{};
	int dash_count{};

	std::deque<float> horizontal_inputs{};
};
} // namespace player
