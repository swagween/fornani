#pragma once
#include <SFML/Graphics.hpp>
#include <fornani/entities/player/abilities/Dash.hpp>
#include <fornani/entities/player/abilities/Jump.hpp>
#include <fornani/entities/player/abilities/Roll.hpp>
#include <fornani/entities/player/abilities/Slide.hpp>
#include <fornani/entities/player/abilities/Wallslide.hpp>
#include <deque>
#include <optional>
#include <unordered_map>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Direction.hpp"

#include <ccmath/ccmath.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {

constexpr static int dash_time{32};
constexpr static int quick_turn_sample_size{24};
constexpr static float backwards_dampen{0.5f};
constexpr static float walk_speed_v{0.62f};
constexpr static float sprint_speed_v{1.0f};
constexpr static float sprint_threshold_v{0.01f};

enum class ControllerInput : std::uint8_t { move_x, jump, sprint, shield, shoot, arms_switch, inspect, dash, move_y, slide };
enum class MovementState : std::uint8_t { restricted, grounded, walking_autonomously, walljumping };
enum class HardState : std::uint8_t { no_move, has_arsenal };

enum class Hook : std::uint8_t { hook_released, hook_held };
enum class Sprint : std::uint8_t { released };

class PlayerController {

  public:
	explicit PlayerController(automa::ServiceProvider& svc);

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
	void reset_vertical_movement();
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
	[[nodiscard]] auto last_requested_direction() -> SimpleDirection const& { return m_last_requested_direction; }

	[[nodiscard]] auto nothing_pressed() -> bool { return key_map[ControllerInput::move_x] == 0.f && key_map[ControllerInput::jump] == 0.f && key_map[ControllerInput::inspect] == 0.f; }
	[[nodiscard]] auto moving() -> bool { return key_map[ControllerInput::move_x] != 0.f; }
	[[nodiscard]] auto sprinting() -> bool { return ccm::abs(key_map[ControllerInput::move_x]) > walk_speed_v + sprint_threshold_v; }
	[[nodiscard]] auto moving_left() -> bool { return key_map[ControllerInput::move_x] < 0.f; }
	[[nodiscard]] auto moving_right() -> bool { return key_map[ControllerInput::move_x] > 0.f; }
	[[nodiscard]] auto facing_left() const -> bool { return direction.lnr == LNR::left; }
	[[nodiscard]] auto facing_right() const -> bool { return direction.lnr == LNR::right; }
	[[nodiscard]] auto restricted() const -> bool { return flags.test(MovementState::restricted); }
	[[nodiscard]] auto grounded() const -> bool { return flags.test(MovementState::grounded); }
	[[nodiscard]] auto is_walljumping() const -> bool { return flags.test(MovementState::walljumping); }
	[[nodiscard]] auto walking_autonomously() const -> bool { return flags.test(MovementState::walking_autonomously); }
	[[nodiscard]] auto dash_requested() const -> bool { return dash_request > -1; }
	[[nodiscard]] auto shot() -> bool { return key_map[ControllerInput::shoot] == 1.f; }
	[[nodiscard]] auto sliding() -> bool { return key_map[ControllerInput::slide] != 0.f; }
	[[nodiscard]] auto is_sprinting() -> bool { return key_map[ControllerInput::move_x] > walk_speed_v; }
	[[nodiscard]] auto released_hook() -> bool {
		auto const ret = hook_flags.test(Hook::hook_released);
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

	[[nodiscard]] auto get_dash_request() const -> int { return dash_request; }
	[[nodiscard]] auto get_dash_count() const -> int { return dash_count; }

	[[nodiscard]] auto vertical_movement() -> float { return key_map[ControllerInput::move_y]; }
	[[nodiscard]] auto horizontal_movement() -> float { return key_map[ControllerInput::move_x]; }
	[[nodiscard]] auto sliding_movement() -> float { return key_map[ControllerInput::slide]; }
	[[nodiscard]] auto arms_switch() -> float { return key_map[ControllerInput::arms_switch]; }
	[[nodiscard]] auto dash_value() -> float { return key_map[ControllerInput::dash]; }

	[[nodiscard]] auto get_jump() -> Jump& { return jump; }
	[[nodiscard]] auto get_wallslide() -> Wallslide& { return wallslide; }
	[[nodiscard]] auto get_slide() -> Slide& { return slide; }
	Roll roll{};

	Direction direction{};

  private:
	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{};	// unused
	util::BitFlags<HardState> hard_state{}; // unused
	util::BitFlags<Sprint> sprint_flags{};
	util::BitFlags<Hook> hook_flags{};

	SimpleDirection m_last_requested_direction{};

	Jump jump{};
	Wallslide wallslide{};
	Slide slide{};

	int dash_request{-1};
	int dash_count{};

	struct {
		util::Cooldown inspect{};
	} cooldowns{};
};
} // namespace fornani::player
