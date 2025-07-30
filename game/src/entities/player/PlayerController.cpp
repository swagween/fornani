
#include "fornani/entities/player/PlayerController.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

constexpr static float crawl_speed_v{0.32f};

PlayerController::PlayerController(automa::ServiceProvider& svc, Player& player) : m_player(&player), cooldowns{.inspect = util::Cooldown(64)}, post_slide{80}, post_wallslide{16} {
	key_map.insert(std::make_pair(ControllerInput::move_x, 0.f));
	key_map.insert(std::make_pair(ControllerInput::sprint, 0.f));
	key_map.insert(std::make_pair(ControllerInput::shoot, 0.f));
	key_map.insert(std::make_pair(ControllerInput::arms_switch, 0.f));
	key_map.insert(std::make_pair(ControllerInput::inspect, 0.f));
	key_map.insert(std::make_pair(ControllerInput::slide, 0.f));
	direction.und = UND::neutral;
	direction.lnr = LNR::right;
}

void PlayerController::update(automa::ServiceProvider& svc, world::Map& map, Player& player) {
	if (walking_autonomously()) {
		prevent_movement();
		key_map[ControllerInput::move_x] = direction.lnr == LNR::left ? -walk_speed_v : walk_speed_v;
	}
	if (hard_state.test(HardState::no_move)) {
		key_map = {};
		return;
	}
	if (walking_autonomously()) { return; }

	auto const& left = svc.controller_map.digital_action_status(config::DigitalAction::platformer_left).held;
	auto const& right = svc.controller_map.digital_action_status(config::DigitalAction::platformer_right).held;
	auto const& up = svc.controller_map.digital_action_status(config::DigitalAction::platformer_up).held;
	auto const& down = svc.controller_map.digital_action_status(config::DigitalAction::platformer_down).held;

	auto sprint = svc.controller_map.digital_action_status(config::DigitalAction::platformer_sprint).held;
	auto sprint_release = svc.controller_map.digital_action_status(config::DigitalAction::platformer_sprint).released;
	auto sprint_pressed = svc.controller_map.digital_action_status(config::DigitalAction::platformer_sprint).triggered;
	if (svc.controller_map.is_autosprint_enabled()) {
		sprint = !sprint;
		sprint_release = sprint_pressed;
	}
	sprint ? input_flags.set(InputState::sprint) : input_flags.reset(InputState::sprint);

	auto const& jump_started = svc.controller_map.digital_action_status(config::DigitalAction::platformer_jump).triggered;
	auto const& jump_held = svc.controller_map.digital_action_status(config::DigitalAction::platformer_jump).held;
	auto const& jump_released = svc.controller_map.digital_action_status(config::DigitalAction::platformer_jump).released;

	auto const& shoot_pressed = svc.controller_map.digital_action_status(config::DigitalAction::platformer_shoot).triggered;
	auto const& shoot_released = svc.controller_map.digital_action_status(config::DigitalAction::platformer_shoot).released;

	auto const& arms_switch_left = svc.controller_map.digital_action_status(config::DigitalAction::platformer_arms_switch_left).triggered;
	auto const& arms_switch_right = svc.controller_map.digital_action_status(config::DigitalAction::platformer_arms_switch_right).triggered;

	auto const& left_released = svc.controller_map.digital_action_status(config::DigitalAction::platformer_left).released;
	auto const& right_released = svc.controller_map.digital_action_status(config::DigitalAction::platformer_right).released;
	auto const& down_released = svc.controller_map.digital_action_status(config::DigitalAction::platformer_down).released;
	auto const& down_pressed = svc.controller_map.digital_action_status(config::DigitalAction::platformer_down).triggered;

	auto it{svc.controller_map.digital_action_status(config::DigitalAction::platformer_inspect).triggered};
	auto ir{svc.controller_map.digital_action_status(config::DigitalAction::platformer_inspect).released};
	auto ih{svc.controller_map.digital_action_status(config::DigitalAction::platformer_inspect).held && cooldowns.inspect.is_almost_complete()};

	auto const& left_pressed = svc.controller_map.digital_action_status(config::DigitalAction::platformer_left).triggered;
	auto const& right_pressed = svc.controller_map.digital_action_status(config::DigitalAction::platformer_right).triggered;

	// set dash direction
	if (up) { m_dash_direction = Direction{{0, 1}}; }
	if (down) { m_dash_direction = Direction{{0, -1}}; }
	if (left) { m_dash_direction = Direction{{-1, 0}}; }
	if (right) { m_dash_direction = Direction{{1, 0}}; }

	// inspect
	auto const& inspected = (it) && grounded() && !left && !right;
	cooldowns.inspect.update();
	if (it) { cooldowns.inspect.start(); }

	auto const& dash_left = svc.controller_map.digital_action_status(config::DigitalAction::platformer_dash).triggered && !grounded() && left;
	auto const& dash_right = svc.controller_map.digital_action_status(config::DigitalAction::platformer_dash).triggered && !grounded() && right;

	key_map[ControllerInput::move_y] = 0.f;
	if (up) { key_map[ControllerInput::move_y] -= 1.f; }
	if (down) { key_map[ControllerInput::move_y] += 1.f; }

	/* handle abilities */
	post_slide.update();
	post_wallslide.update();
	if (player.grounded()) { player.m_ability_usage = {}; }
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_dash).triggered) {
		if (player.can_dash()) {
			m_ability = std::make_unique<Dash>(svc, map, player.collider, m_dash_direction, player.can_omnidirectional_dash());
			player.m_ability_usage.dash.update();
		}
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_jump).triggered) {
		if (player.can_jump()) { m_ability = std::make_unique<Jump>(svc, map, player.collider); }
		if (player.can_doublejump()) {
			m_ability = std::make_unique<Doublejump>(svc, map, player.collider);
			player.m_ability_usage.doublejump.update();
		}
		if (is_wallsliding()) { m_ability = std::make_unique<Walljump>(svc, map, player.collider, player.get_actual_direction()); }
	}

	// crouching, rolling, and sliding
	flags.reset(MovementState::crouch);
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_slide).held) {
		if (!grounded()) { input_flags.set(InputState::slide_in_air); }
		if (!m_ability && player.can_slide() && sprint && !post_slide.running() && moving()) { m_ability = std::make_unique<Slide>(svc, map, player.collider, player.get_actual_direction()); }
		if (!sprint) { flags.set(MovementState::crouch); }
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_slide).triggered) {
		if (!m_ability && player.can_roll() && sprint) { m_ability = std::make_unique<Roll>(svc, map, player.collider, player.get_actual_direction()); }
	}
	if (m_ability) {
		if (m_ability.value()->is(AbilityType::roll)) { input_flags.reset(InputState::slide_in_air); }
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_slide).released) { input_flags.reset(InputState::slide_in_air); }

	// wallslide
	if ((left && player.collider.has_left_wallslide_collision()) || (right && player.collider.has_right_wallslide_collision())) {
		if (player.can_wallslide() && !post_wallslide.running()) {
			if (!is(AbilityType::walljump)) { m_ability = std::make_unique<Wallslide>(svc, map, player.collider, player.get_actual_direction()); }
		}
	}

	if (m_ability) {
		m_ability.value()->update(player.collider, *this);

		// stop rising if player releases jump control
		if (is(AbilityType::jump) || is(AbilityType::doublejump) || is(AbilityType::walljump)) {
			if (svc.controller_map.digital_action_status(config::DigitalAction::platformer_jump).released) { m_ability.value()->cancel(); }
			if (m_ability.value()->cancelled() && player.collider.physics.apparent_velocity().y < 0.0f) {
				player.collider.physics.acceleration.y *= player.physics_stats.jump_release_multiplier;
				m_ability.value()->fail();
			}
		}
		if (m_ability.value()->is(AbilityType::slide) && svc.controller_map.digital_action_status(config::DigitalAction::platformer_slide).released) {
			m_ability.value()->fail();
			post_slide.start();
		}
		if (m_ability.value()->is_done() || m_ability.value()->failed()) { m_ability = {}; }
	}
	/* end abilities */

	key_map[ControllerInput::move_x] = 0.f;
	// keyboard
	if (svc.controller_map.is_gamepad()) {
		key_map[ControllerInput::move_x] = svc.controller_map.get_joystick_throttle().x;
	} else {
		if (left) { key_map[ControllerInput::move_x] -= is_crouching() && grounded() ? crawl_speed_v : walk_speed_v; }
		if (right) { key_map[ControllerInput::move_x] += is_crouching() && grounded() ? crawl_speed_v : walk_speed_v; }
	}

	// sprint
	key_map[ControllerInput::sprint] = 0.f;
	if (moving() && sprint && !sprint_released()) {
		if (left) { key_map[ControllerInput::move_x] = -sprint_speed_v; }
		if (right) { key_map[ControllerInput::move_x] = sprint_speed_v; }
		key_map[ControllerInput::sprint] = key_map[ControllerInput::move_x];
	}

	direction.set_intermediate(left, right, up, down);

	// sprint
	if (sprint_release) { sprint_flags.set(Sprint::released); }
	if (grounded()) { sprint_flags = {}; }

	if (shoot_pressed) { key_map[ControllerInput::shoot] = 1.f; }
	if (shoot_released) { key_map[ControllerInput::shoot] = 0.f; }

	bool firing_automatic = false;
	if (!restricted() && (!shot() || !has_arsenal())) {
		direction.lnr = moving_left() ? LNR::left : direction.lnr;
		direction.lnr = moving_right() ? LNR::right : direction.lnr;
		direction.und = UND::neutral;
		direction.und = up ? UND::up : direction.und;
		direction.und = down && !grounded() ? UND::down : direction.und;
	} else if (((moving_left() && direction.lnr == LNR::right) || (moving_right() && direction.lnr == LNR::left)) && has_arsenal()) {
		key_map[ControllerInput::move_x] *= backwards_dampen;
		firing_automatic = true;
	} else if (((moving_left() && direction.lnr == LNR::right) || (moving_right() && direction.lnr == LNR::left))) {
		key_map[ControllerInput::slide] = 0.f;
	}

	if ((left_pressed || left) && !firing_automatic && !is_crouching()) { m_last_requested_direction.set(LR::left); }
	if ((right_pressed || right) && !firing_automatic && !is_crouching()) { m_last_requested_direction.set(LR::right); }

	key_map[ControllerInput::arms_switch] = 0.f;
	key_map[ControllerInput::arms_switch] = arms_switch_left ? -1.f : key_map[ControllerInput::arms_switch];
	key_map[ControllerInput::arms_switch] = arms_switch_right ? 1.f : key_map[ControllerInput::arms_switch];

	key_map[ControllerInput::inspect] = inspected ? 1.f : 0.f;
}

void PlayerController::clean() { flags = {}; }

void PlayerController::stop() { key_map[ControllerInput::move_x] = 0.f; }

void PlayerController::restrict_movement() {
	flags.set(MovementState::restricted);
	hard_state.set(HardState::no_move);
}

void PlayerController::unrestrict() {
	flags.reset(MovementState::restricted);
	hard_state.reset(HardState::no_move);
}

void PlayerController::uninspect() { key_map[ControllerInput::inspect] = 0.f; }

void player::PlayerController::reset_vertical_movement() { key_map[ControllerInput::move_y] = 0.f; }

void PlayerController::walljump() { flags.set(MovementState::walljumping); }

void PlayerController::autonomous_walk() {
	direction.lnr == LNR::right ? key_map[ControllerInput::move_x] = walk_speed_v : key_map[ControllerInput::move_x] = -walk_speed_v;
	if (sprinting()) { key_map[ControllerInput::sprint] = key_map[ControllerInput::move_x]; }
	flags.set(MovementState::walking_autonomously);
}

void PlayerController::stop_walking_autonomously() { flags.reset(MovementState::walking_autonomously); }

void PlayerController::set_shot(bool flag) { key_map[ControllerInput::shoot] = flag ? 1.f : 0.f; }

void PlayerController::prevent_movement() {
	key_map[ControllerInput::move_x] = 0.f;
	key_map[ControllerInput::move_y] = 0.f;
	key_map[ControllerInput::arms_switch] = 0.f;
	key_map[ControllerInput::inspect] = 0.f;
	key_map[ControllerInput::shoot] = 0.f;
	key_map[ControllerInput::sprint] = 0.f;
	key_map[ControllerInput::slide] = 0.f;
	flags.set(MovementState::restricted);
}

void PlayerController::stop_walljumping() { flags.reset(MovementState::walljumping); }

void PlayerController::set_arsenal(bool const has) { has ? hard_state.set(HardState::has_arsenal) : hard_state.reset(HardState::has_arsenal); }

std::optional<float> PlayerController::get_controller_state(ControllerInput key) const {
	if (auto search = key_map.find(key); search != key_map.end()) {
		return search->second;
	} else {
		return std::nullopt;
	}
}
std::optional<AnimState> PlayerController::get_ability_animation() const {
	if (m_ability) { return m_ability.value()->get_animation(); }
	return std::nullopt;
}

bool PlayerController::grounded() const { return m_player->grounded(); }

} // namespace fornani::player
