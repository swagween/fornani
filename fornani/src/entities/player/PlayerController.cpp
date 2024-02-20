#include "PlayerController.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace controllers {

PlayerController::PlayerController() {
	key_map.insert(std::make_pair(Movement::move_x, 0.f));
	key_map.insert(std::make_pair(Movement::jump, 0.f));
}

void PlayerController::update() {

	auto const& left = svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::held);
	auto const& right = svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::held);
	auto const& start_jump = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered);


	key_map[Movement::move_x] = 0.f;
	key_map[Movement::move_x] = left && !right ? -1.f : key_map[Movement::move_x];
	key_map[Movement::move_x] = right && !left ? 1.f : key_map[Movement::move_x];
	key_map[Movement::move_x] = right && left ? 0.f : key_map[Movement::move_x];

	direction = moving_left() ? Direction::left : direction;
	direction = moving_right() ? Direction::right : direction;

	key_map[Movement::jump] = start_jump ? 1.f : 0.f;
	if (start_jump) { jump(); }
	if (!flags.test(MovementState::grounded)) { prevent_jump(); }

}

void PlayerController::jump() {
	if (!flags.test(MovementState::restricted)) {
		jump_flags.set(Jump::is_pressed);
		jump_request = jump_time;
		jump_flags.set(Jump::just_jumped);
		jump_flags.set(Jump::trigger);
	}
}

void PlayerController::sustain_jump() {
	if (jump_flags.test(Jump::just_jumped) || jump_flags.test(Jump::hold) || jump_flags.test(Jump::jumping) || jump_request > -1) { jump_flags.set(Jump::is_released); }
	jump_flags.reset(Jump::is_pressed);
	jump_flags.reset(Jump::hold);
	if (!restricted()) { jump_flags.set(Jump::can_jump); }
}

void PlayerController::prevent_jump() { jump_request = -1; }

void PlayerController::stop() {
	key_map[Movement::move_x] = 0.f;
	key_map[Movement::jump] = 0.f;
}

void PlayerController::ground() { flags.set(MovementState::grounded); }

void PlayerController::unground() { flags.reset(MovementState::grounded); }

void PlayerController::start_jumping() {
	jump_flags.set(Jump::jumping);
	jump_flags.reset(Jump::can_jump);
	jump_flags.reset(Jump::trigger);
}

void PlayerController::set_jump_hold() { jump_flags.set(Jump::hold); }

void PlayerController::reset_jump() { jump_flags.reset(Jump::jumping); }

void PlayerController::reset_just_jumped() { jump_flags.reset(Jump::just_jumped); }

void PlayerController::reset_jump_flags() {
	if (!jump_flags.test(Jump::is_pressed)) { jump_flags.reset(Jump::hold); }
	if (flags.test(MovementState::grounded) && jump_request == -1) { jump_flags.reset(Jump::is_released); }

	if (jump_flags.test(Jump::is_released) && !flags.test(MovementState::grounded)) { jump_flags.reset(Jump::is_released); }
}

void PlayerController::decrement_jump() {
	dt = svc::clockLocator.get().tick_rate;

	auto new_time = Clock::now();
	Time frame_time = std::chrono::duration_cast<Time>(new_time - current_time);

	if (frame_time.count() > svc::clockLocator.get().frame_limit) { frame_time = Time{svc::clockLocator.get().frame_limit}; }
	current_time = new_time;
	accumulator += frame_time;

	int integrations = 0;
	while (accumulator >= dt) {

		--jump_request;

		accumulator -= dt;
		++integrations;
	}
}

float& PlayerController::get_controller_state(Movement key) { return key_map[key]; }

bool PlayerController::moving() { return key_map[Movement::move_x] != 0.f; }

bool PlayerController::moving_left() { return key_map[Movement::move_x] < 0.f; }

bool PlayerController::moving_right() { return key_map[Movement::move_x] > 0.f; }

bool PlayerController::facing_left() const { return direction == Direction::left; }

bool PlayerController::facing_right() const { return direction == Direction::right; }

bool PlayerController::restricted() const { return flags.test(MovementState::restricted); }

bool PlayerController::jump_requested() const { return jump_request > -1; }

bool PlayerController::just_jumped() { return jump_flags.test(Jump::just_jumped); }

bool PlayerController::jump_triggered() const { return jump_flags.test(Jump::trigger); }

bool PlayerController::jump_released() const { return jump_flags.test(Jump::is_released) && jump_flags.test(Jump::jumping); }

} // namespace controllers
