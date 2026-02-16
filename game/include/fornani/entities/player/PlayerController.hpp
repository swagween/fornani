
#pragma once

#include <SFML/Graphics.hpp>
#include <ccmath/ccmath.hpp>
#include <fornani/entities/player/abilities/Ability.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <optional>
#include <unordered_map>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {

class Player;

constexpr static int dash_time{32};
constexpr static int quick_turn_sample_size{24};
constexpr static float backwards_dampen{0.5f};
constexpr static float walk_speed_v{0.62f};
constexpr static float sprint_speed_v{1.0f};
constexpr static float sprint_threshold_v{0.01f};

enum class PlayerControllerFlags { firing_weapon };
enum class ControllerInput { move_x, sprint, shoot, arms_switch, inspect, move_y, slide };
enum class MovementState { restricted, walljumping, crouch };
enum class HardState { no_move, has_arsenal, walking_autonomously };
enum class InputState { slide_in_air, sprint };

enum class Sprint { released };

class PlayerController final : public Flaggable<PlayerControllerFlags> {

  public:
	friend class Player;
	explicit PlayerController(automa::ServiceProvider& svc, Player& player);

	void update(automa::ServiceProvider& svc, world::Map& map, Player& player);
	void clean();
	void stop();
	void restrict_movement();
	void unrestrict();
	void uninspect();
	void reset_vertical_movement();
	void walljump();
	void autonomous_walk();
	void stop_walking_autonomously();
	void set_shot(bool flag);
	void prevent_movement();
	void stop_walljumping();
	void set_arsenal(bool const has);
	void set_direction(Direction to);

	std::optional<float> get_controller_state(ControllerInput key) const;
	std::optional<AnimState> get_ability_animation() const;
	[[nodiscard]] auto last_requested_direction() -> SimpleDirection const& { return m_last_requested_direction; }
	[[nodiscard]] auto can_move() const -> bool { return !hard_state.test(HardState::no_move); }
	[[nodiscard]] auto is(AbilityType type) const -> bool { return m_ability ? m_ability.value()->is(type) : false; }
	[[nodiscard]] auto is_dashing() const -> bool { return m_ability ? m_ability.value()->is(AbilityType::dash) : false; }
	[[nodiscard]] auto is_sliding() const -> bool { return m_ability ? m_ability.value()->is(AbilityType::slide) : false; }
	[[nodiscard]] auto is_wallsliding() const -> bool { return m_ability ? m_ability.value()->is(AbilityType::wallslide) : false; }
	[[nodiscard]] auto is_rolling() const -> bool { return m_ability ? m_ability.value()->is(AbilityType::roll) && m_ability.value()->is_active() : false; }
	[[nodiscard]] auto slid_in_air() const -> bool { return input_flags.test(InputState::slide_in_air); }
	[[nodiscard]] auto is_crouching() const -> bool { return flags.test(MovementState::crouch); }
	[[nodiscard]] auto is_ability_active() const -> bool { return m_ability ? m_ability.value()->is_active() : false; }
	[[nodiscard]] auto is_ability_cancelled() const -> bool { return m_ability ? m_ability.value()->cancelled() : false; }
	[[nodiscard]] auto is_animation_request() const -> bool { return m_ability ? m_ability.value()->is_animation_request() : false; }
	[[nodiscard]] auto get_ability_type() const -> std::optional<AbilityType> {
		if (m_ability) { return m_ability.value()->get_type(); }
		return std::nullopt;
	}
	[[nodiscard]] auto get_ability_direction() const -> Direction { return m_ability ? m_ability.value()->get_direction() : Direction{}; }

	[[nodiscard]] auto nothing_pressed() -> bool { return key_map[ControllerInput::move_x] == 0.f && key_map[ControllerInput::inspect] == 0.f; }
	[[nodiscard]] auto moving() -> bool { return key_map[ControllerInput::move_x] != 0.f; }
	[[nodiscard]] auto sprinting() -> bool { return ccm::abs(key_map[ControllerInput::move_x]) > walk_speed_v + sprint_threshold_v; }
	[[nodiscard]] auto sprint_held() -> bool { return input_flags.test(InputState::sprint); }
	[[nodiscard]] auto moving_left() -> bool { return key_map[ControllerInput::move_x] < 0.f; }
	[[nodiscard]] auto moving_right() -> bool { return key_map[ControllerInput::move_x] > 0.f; }
	[[nodiscard]] auto facing_left() const -> bool { return direction.lnr == LNR::left; }
	[[nodiscard]] auto facing_right() const -> bool { return direction.lnr == LNR::right; }
	[[nodiscard]] auto restricted() const -> bool { return flags.test(MovementState::restricted); }
	[[nodiscard]] auto grounded() const -> bool;
	[[nodiscard]] auto is_walljumping() const -> bool { return flags.test(MovementState::walljumping); }
	[[nodiscard]] auto walking_autonomously() const -> bool { return hard_state.test(HardState::walking_autonomously); }
	[[nodiscard]] auto shot() -> bool { return key_map[ControllerInput::shoot] == 1.f; }
	[[nodiscard]] auto is_sprinting() -> bool { return ccm::abs(key_map[ControllerInput::move_x]) > walk_speed_v; }
	[[nodiscard]] auto has_arsenal() const -> bool { return hard_state.test(HardState::has_arsenal); }
	[[nodiscard]] auto inspecting() -> bool { return key_map[ControllerInput::inspect] == 1.f; }
	[[nodiscard]] auto sprint_released() const -> bool { return sprint_flags.test(Sprint::released); }

	[[nodiscard]] auto vertical_movement() -> float { return key_map[ControllerInput::move_y]; }
	[[nodiscard]] auto horizontal_movement() -> float { return key_map[ControllerInput::move_x]; }
	[[nodiscard]] auto sliding_movement() -> float { return key_map[ControllerInput::slide]; }
	[[nodiscard]] auto arms_switch() -> float { return key_map[ControllerInput::arms_switch]; }

	Direction direction{};

	util::Cooldown post_slide;
	util::Cooldown post_wallslide;
	util::Cooldown wallslide_slowdown;

  private:
	void flush_ability() { m_ability = {}; }
	Direction m_dash_direction{};

	std::unordered_map<ControllerInput, float> key_map{};
	util::BitFlags<MovementState> flags{};	// unused
	util::BitFlags<HardState> hard_state{}; // unused
	util::BitFlags<Sprint> sprint_flags{};
	util::BitFlags<InputState> input_flags{};

	SimpleDirection m_last_requested_direction{};

	std::optional<std::unique_ptr<Ability>> m_ability{};

	struct {
		util::Cooldown inspect{};
		util::Cooldown dash_kick{};
	} cooldowns{};

	Player* m_player;

	io::Logger m_logger{"Controller"};
};

} // namespace fornani::player
