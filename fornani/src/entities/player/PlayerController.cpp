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
	auto const& jump_started = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered);
	auto const& jump_held = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::held);
	auto const& jump_released = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::released);


	key_map[Movement::move_x] = 0.f;
	key_map[Movement::move_x] = left && !right ? -1.f : key_map[Movement::move_x];
	key_map[Movement::move_x] = right && !left ? 1.f : key_map[Movement::move_x];
	key_map[Movement::move_x] = right && left ? 0.f : key_map[Movement::move_x];

	direction = moving_left() ? Direction::left : direction;
	direction = moving_right() ? Direction::right : direction;

	key_map[Movement::jump] = jump_started ? 1.f : 0.f;

	bool can_launch = !restricted() && flags.test(MovementState::grounded) && !jump_flags.test(Jump::jumping);
	can_launch ? jump_flags.set(Jump::can_jump) : jump_flags.reset(Jump::can_jump);

	if (jump_started) { jump_request = jump_time; }
	if (jump_held) {
	}
	if (jump_released) { jump_flags.set(Jump::is_released); }

	if (jump_requested() && flags.test(MovementState::grounded)) {
		jump_flags.set(Jump::jumpsquat_trigger);
		prevent_jump();
	}


}

void PlayerController::jump() { jump_flags.set(Jump::jumping); }

void PlayerController::prevent_jump() { jump_request = -1; }

void PlayerController::stop() {
	key_map[Movement::move_x] = 0.f;
	key_map[Movement::jump] = 0.f;
}

void PlayerController::ground() { flags.set(MovementState::grounded); }

void PlayerController::unground() { flags.reset(MovementState::grounded); }

void PlayerController::restrict() { flags.set(MovementState::restricted); }

void PlayerController::unrestrict() { flags.reset(MovementState::restricted); }

void PlayerController::start_jumping() {
	jump_flags.set(Jump::jumping);
	jump_flags.reset(Jump::can_jump);
	jump_flags.reset(Jump::trigger);
}

void PlayerController::reset_jump() {
	jump_flags.reset(Jump::jumping);
	jump_flags.reset(Jump::is_released);
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

	if (jump_request < 0) { jump_request = -1; }

}

void PlayerController::start_jumpsquat() {
	jump_flags.set(Jump::jumpsquat_trigger);
	jump_flags.set(Jump::jumpsquatting);
}

void PlayerController::stop_jumpsquatting() { jump_flags.reset(Jump::jumpsquatting); }

void PlayerController::reset_jumpsquat_trigger() { jump_flags.reset(Jump::jumpsquat_trigger); }

void PlayerController::reset_just_jumped() { jump_flags.reset(Jump::just_jumped); }

std::optional<float> PlayerController::get_controller_state(Movement key) const {
	if (auto search = key_map.find(key); search != key_map.end()) {
		return search->second;
	} else {
		return std::nullopt;
	}
}

bool PlayerController::moving() { return key_map[Movement::move_x] != 0.f; }

bool PlayerController::moving_left() { return key_map[Movement::move_x] < 0.f; }

bool PlayerController::moving_right() { return key_map[Movement::move_x] > 0.f; }

bool PlayerController::facing_left() const { return direction == Direction::left; }

bool PlayerController::facing_right() const { return direction == Direction::right; }

bool PlayerController::restricted() const { return flags.test(MovementState::restricted); }

bool PlayerController::jump_requested() const { return jump_request > -1; }

bool PlayerController::jump_released() const { return jump_flags.test(Jump::is_released) && jump_flags.test(Jump::jumping); }

bool PlayerController::can_jump() const { return jump_flags.test(Jump::can_jump); }

bool PlayerController::jumping() const { return jump_flags.test(Jump::jumping); }

bool PlayerController::just_jumped() const { return jump_flags.test(Jump::just_jumped); }

bool PlayerController::jumpsquatting() const { return jump_flags.test(Jump::jumpsquatting); }

bool PlayerController::jumpsquat_trigger() const { return jump_flags.test(Jump::jumpsquat_trigger); }

int PlayerController::get_jump_request() const { return jump_request; }

} // namespace controllers
