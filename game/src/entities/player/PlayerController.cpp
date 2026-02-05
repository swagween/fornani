
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/player/PlayerController.hpp>
#include <fornani/entities/player/abilities/Dive.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::player {

constexpr static float crawl_speed_v{0.32f};

PlayerController::PlayerController(automa::ServiceProvider& svc, Player& player) : m_player(&player), cooldowns{.inspect{64}, .dash_kick{134}}, post_slide{80}, post_wallslide{16}, wallslide_slowdown{64} {
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

	auto sprint = svc.input_system.digital(input::DigitalAction::sprint).held;
	auto sprint_release = svc.input_system.digital(input::DigitalAction::sprint).released;
	auto sprint_pressed = svc.input_system.digital(input::DigitalAction::sprint).triggered;
	if (svc.input_system.is_autosprint_enabled() && !svc.input_system.is_gamepad()) {
		sprint = !sprint;
		sprint_release = sprint_pressed;
	}
	sprint ? input_flags.set(InputState::sprint) : input_flags.reset(InputState::sprint);

	if (walking_autonomously()) {
		auto speed = sprint ? sprint_speed_v : walk_speed_v;
		key_map[ControllerInput::move_x] = direction.lnr == LNR::left ? -speed : speed;
	}
	if (hard_state.test(HardState::no_move)) {
		key_map = {};
		return;
	}
	if (walking_autonomously()) { return; }
	if (svc.state_flags.test(automa::StateFlags::cutscene)) { return; }

	auto dash_and_jump_combined = svc.input_system.is_bound_to_same_input(input::DigitalAction::dash, input::DigitalAction::jump);

	auto const left = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::left);
	auto const right = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::right);
	auto const up = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::up);
	auto const down = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::down);
	auto const any_direction_held = left || right || up || down;

	auto const& jump_started = svc.input_system.digital(input::DigitalAction::jump).triggered;
	auto const& jump_held = svc.input_system.digital(input::DigitalAction::jump).held;
	auto const& jump_released = svc.input_system.digital(input::DigitalAction::jump).released;

	auto const& shoot_pressed = svc.input_system.digital(input::DigitalAction::shoot).triggered;
	auto const& shoot_released = svc.input_system.digital(input::DigitalAction::shoot).released;

	auto const& arms_switch_left = svc.input_system.digital(input::DigitalAction::tab_left).triggered;
	auto const& arms_switch_right = svc.input_system.digital(input::DigitalAction::tab_right).triggered;

	auto const left_released = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::left);
	auto const right_released = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::right);
	auto const down_released = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::down);
	auto const down_pressed = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::down);

	auto it{svc.input_system.digital(input::DigitalAction::inspect).triggered};
	auto ir{svc.input_system.digital(input::DigitalAction::inspect).released};
	auto ih{svc.input_system.digital(input::DigitalAction::inspect).held && cooldowns.inspect.is_almost_complete()};

	auto const left_pressed = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::left);
	auto const right_pressed = svc.input_system.direction_held(input::AnalogAction::move, input::MoveDirection::right);

	// set dash direction
	if (up) { m_dash_direction = Direction{{0, 1}}; }
	if (down) { m_dash_direction = Direction{{0, -1}}; }
	if (left) { m_dash_direction = Direction{{-1, 0}}; }
	if (right) { m_dash_direction = Direction{{1, 0}}; }

	// inspect
	auto const& inspected = (it) && grounded() && !left && !right;
	cooldowns.inspect.update();
	if (it) { cooldowns.inspect.start(); }

	cooldowns.dash_kick.update();

	auto const& dash_left = svc.input_system.digital(input::DigitalAction::dash).triggered && !grounded() && left;
	auto const& dash_right = svc.input_system.digital(input::DigitalAction::dash).triggered && !grounded() && right;

	key_map[ControllerInput::move_y] = 0.f;
	if (up) { key_map[ControllerInput::move_y] -= 1.f; }
	if (down) { key_map[ControllerInput::move_y] += 1.f; }

	/* handle abilities */
	post_slide.update();
	post_wallslide.update();
	if (player.grounded()) { player.m_ability_usage = {}; }
	if (svc.input_system.digital(input::DigitalAction::dash).triggered) {
		auto const dj_guard = (dash_and_jump_combined && any_direction_held) || !dash_and_jump_combined;
		if (player.can_dash() && !is_wallsliding() && dj_guard) {
			m_ability = std::make_unique<Dash>(svc, map, player.get_collider(), m_dash_direction, player.can_omnidirectional_dash());
			player.m_ability_usage.dash.update();
		}
	}
	if (svc.input_system.digital(input::DigitalAction::jump).triggered) {
		if (player.can_jump()) { m_ability = std::make_unique<Jump>(svc, map, player.get_collider()); }
		// guard for when player has jump and dash bound to the same key
		auto const dash_exhausted = !player.can_dash() && !is_dashing();
		auto direction_held = left || right;
		auto can_walljump = (player.get_collider().has_right_wallslide_collision() || player.get_collider().has_left_wallslide_collision()) && !player.get_collider().grounded() && player.can_walljump() && direction_held;
		auto can_doublejump = (player.can_doublejump() && !dash_and_jump_combined) || (player.can_doublejump() && dash_and_jump_combined && (!any_direction_held || dash_exhausted));
		auto jump_direction = player.get_collider().has_right_wallslide_collision() ? Direction{LR::right} : player.get_collider().has_left_wallslide_collision() ? Direction{LR::left} : Direction{};
		if (can_walljump) {
			m_ability = std::make_unique<Walljump>(svc, map, player.get_collider(), jump_direction);
		} else if (can_doublejump && !player.can_dive()) {
			m_ability = std::make_unique<Doublejump>(svc, map, player.get_collider());
			player.m_ability_usage.doublejump.update();
		}
		if (player.can_dive() && !can_walljump) { m_ability = std::make_unique<Dive>(svc, map, player.get_collider()); }
	}
	if (!is_wallsliding()) { svc.soundboard.flags.player.reset(audio::Player::wallslide); }

	// crouching, rolling, and sliding
	flags.reset(MovementState::crouch);
	if (svc.input_system.digital(input::DigitalAction::slide).held) {
		if (!grounded()) { input_flags.set(InputState::slide_in_air); }
		if (!m_ability && player.can_slide() && sprint && !post_slide.running() && moving()) { m_ability = std::make_unique<Slide>(svc, map, player.get_collider(), player.get_actual_direction()); }
		if (!sprint) { flags.set(MovementState::crouch); }
	}
	if (svc.input_system.digital(input::DigitalAction::slide).triggered) {
		auto can_roll = !m_ability;
		if (is(AbilityType::dash) && player.get_collider().grounded()) { can_roll = true; }
		if (can_roll && player.can_roll() && sprint) { m_ability = std::make_unique<Roll>(svc, map, player.get_collider(), player.get_actual_direction()); }
	}
	if (m_ability) {
		if (m_ability.value()->is(AbilityType::roll)) { input_flags.reset(InputState::slide_in_air); }
	}
	if (svc.input_system.digital(input::DigitalAction::slide).released) { input_flags.reset(InputState::slide_in_air); }

	// wallslide
	if ((left && player.get_collider().has_left_wallslide_collision()) || (right && player.get_collider().has_right_wallslide_collision())) {
		if (player.can_wallslide() && !post_wallslide.running()) {
			if (!is(AbilityType::walljump)) {
				if (!is(AbilityType::wallslide)) { wallslide_slowdown.start(); }
				m_ability = std::make_unique<Wallslide>(svc, map, player.get_collider(), player.get_actual_direction());
			}
		}
	}
	if (!is(AbilityType::wallslide)) { player.get_collider().physics.maximum_velocity.y = player.physics_stats.maximum_velocity.y; }
	wallslide_slowdown.update();

	if (player.can_dash_kick() && !cooldowns.dash_kick.running()) {
		m_ability = std::make_unique<DashKick>(svc, map, player.get_collider(), player.get_actual_direction());
		svc.ticker.freeze_frame(8);
		player.health.invincibility.start(16);
		player.m_ability_usage.dash.update(-1);
		player.set_flag(PlayerFlags::dash_kick, false);
		cooldowns.dash_kick.start();
	}

	if (m_ability) {
		m_ability.value()->update(player.get_collider(), *this);

		// stop rising if player releases jump control
		if (is(AbilityType::jump) || is(AbilityType::doublejump) || is(AbilityType::walljump) || is(AbilityType::dive)) {
			if (svc.input_system.digital(input::DigitalAction::jump).released) { m_ability.value()->cancel(); }
			if (m_ability.value()->cancelled() && player.get_collider().physics.apparent_velocity().y < 0.0f) {
				player.get_collider().physics.acceleration.y *= player.physics_stats.jump_release_multiplier;
				m_ability.value()->fail();
			}
		}
		if (m_ability.value()->is(AbilityType::slide) && svc.input_system.digital(input::DigitalAction::slide).released) {
			m_ability.value()->fail();
			post_slide.start();
		}
		if (m_ability.value()->is_done() || m_ability.value()->failed()) { m_ability = {}; }
	}
	/* end abilities */

	// horizontal movement
	key_map[ControllerInput::move_x] = 0.f;
	if (svc.input_system.is_gamepad()) {
		key_map[ControllerInput::move_x] = is_crouching() && grounded() ? crawl_speed_v * svc.input_system.get_joystick_throttle().x : svc.input_system.get_joystick_throttle().x;
	} else {
		if (left) { key_map[ControllerInput::move_x] -= is_crouching() && grounded() ? crawl_speed_v : walk_speed_v; }
		if (right) { key_map[ControllerInput::move_x] += is_crouching() && grounded() ? crawl_speed_v : walk_speed_v; }
	}

	// sprint
	key_map[ControllerInput::sprint] = 0.f;
	if (moving() && sprint && !sprint_released()) {
		if (svc.input_system.is_gamepad()) {
			key_map[ControllerInput::move_x] = svc.input_system.get_joystick_throttle().x;
		} else {
			if (left) { key_map[ControllerInput::move_x] = -sprint_speed_v; }
			if (right) { key_map[ControllerInput::move_x] = sprint_speed_v; }
		}
		key_map[ControllerInput::sprint] = key_map[ControllerInput::move_x];
	}

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
	if (inspected) { NANI_LOG_DEBUG(m_logger, "Inspected!"); }
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

void PlayerController::autonomous_walk() { hard_state.set(HardState::walking_autonomously); }

void PlayerController::stop_walking_autonomously() { hard_state.reset(HardState::walking_autonomously); }

void PlayerController::set_shot(bool flag) { key_map[ControllerInput::shoot] = flag ? 1.f : 0.f; }

void PlayerController::prevent_movement() {
	flush_ability();
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

void PlayerController::set_direction(Direction to) {
	direction = to;
	m_last_requested_direction.set(to.lnr);
}

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
