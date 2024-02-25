#include "PlayerController.hpp"
#include "../../setup/ServiceLocator.hpp"

namespace controllers {

PlayerController::PlayerController() {
	key_map.insert(std::make_pair(ControllerInput::move_x, 0.f));
	key_map.insert(std::make_pair(ControllerInput::jump, 0.f));
	key_map.insert(std::make_pair(ControllerInput::shoot, 0.f));
	key_map.insert(std::make_pair(ControllerInput::arms_switch, 0.f));
	key_map.insert(std::make_pair(ControllerInput::inspect, 0.f));
	direction.und = dir::UND::neutral;
	direction.lr = dir::LR::right;
}

void PlayerController::update() {

	auto const& left = svc::inputStateLocator.get().keys.at(sf::Keyboard::Left).key_state.test(util::key_state::held);
	auto const& right = svc::inputStateLocator.get().keys.at(sf::Keyboard::Right).key_state.test(util::key_state::held);
	auto const& up = svc::inputStateLocator.get().keys.at(sf::Keyboard::Up).key_state.test(util::key_state::held);
	auto const& down = svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::held);
	auto const& jump_started = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::triggered);
	auto const& jump_held = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::held);
	auto const& jump_released = svc::inputStateLocator.get().keys.at(sf::Keyboard::Z).key_state.test(util::key_state::released);
	auto const& shoot_pressed = svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::triggered);
	auto const& shoot_released = svc::inputStateLocator.get().keys.at(sf::Keyboard::X).key_state.test(util::key_state::released);
	auto const& arms_switch_left = svc::inputStateLocator.get().keys.at(sf::Keyboard::A).key_state.test(util::key_state::triggered);
	auto const& arms_switch_right = svc::inputStateLocator.get().keys.at(sf::Keyboard::S).key_state.test(util::key_state::triggered);
	auto const& inspected = svc::inputStateLocator.get().keys.at(sf::Keyboard::Down).key_state.test(util::key_state::triggered) && grounded();

	key_map[ControllerInput::move_x] = 0.f;
	key_map[ControllerInput::move_x] = left && !right ? -1.f : key_map[ControllerInput::move_x];
	key_map[ControllerInput::move_x] = right && !left ? 1.f : key_map[ControllerInput::move_x];
	key_map[ControllerInput::move_x] = right && left ? 0.f : key_map[ControllerInput::move_x];

	direction.lr = moving_left() ? dir::LR::left : direction.lr;
	direction.lr = moving_right() ? dir::LR::right : direction.lr;
	direction.und = dir::UND::neutral;
	direction.und = up ? dir::UND::up : direction.und;
	direction.und = down ? dir::UND::down : direction.und;

	key_map[ControllerInput::jump] = jump_started ? 1.f : 0.f;

	if (shoot_pressed) { key_map[ControllerInput::shoot] = 1.f; }
	if (shoot_released) { key_map[ControllerInput::shoot] = 0.f; }

	key_map[ControllerInput::arms_switch] = 0.f;
	key_map[ControllerInput::arms_switch] = arms_switch_left ? -1.f : key_map[ControllerInput::arms_switch];
	key_map[ControllerInput::arms_switch] = arms_switch_right ? 1.f : key_map[ControllerInput::arms_switch];

	key_map[ControllerInput::inspect] = inspected ? 1.f : 0.f;

	bool can_launch = !restricted() && flags.test(MovementState::grounded) && !jump_flags.test(Jump::jumping);
	can_launch ? jump_flags.set(Jump::can_jump) : jump_flags.reset(Jump::can_jump);

	if (jump_started) { jump_request = jump_time; }
	if (jump_held) {
		jump_flags.set(Jump::jump_held);
	} else {
		jump_flags.reset(Jump::jump_held);
	}
	if (jump_released) { jump_flags.set(Jump::is_released); }

	if (jump_requested() && flags.test(MovementState::grounded)) {
		jump_flags.set(Jump::jumpsquat_trigger);
		prevent_jump();
	}

	// svc::tickerLocator.get().tick([this] { decrement_jump(); });
	decrement_jump();
}

void PlayerController::jump() { jump_flags.set(Jump::jumping); }

void PlayerController::prevent_jump() { jump_request = -1; }

void PlayerController::stop() {
	key_map[ControllerInput::move_x] = 0.f;
	key_map[ControllerInput::jump] = 0.f;
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
	--jump_request;
	if (jump_request < 0) { jump_request = -1; }
}

void PlayerController::start_jumpsquat() {
	jump_flags.set(Jump::jumpsquat_trigger);
	jump_flags.set(Jump::jumpsquatting);
}

void PlayerController::stop_jumpsquatting() { jump_flags.reset(Jump::jumpsquatting); }

void PlayerController::reset_jumpsquat_trigger() { jump_flags.reset(Jump::jumpsquat_trigger); }

void PlayerController::reset_just_jumped() { jump_flags.reset(Jump::just_jumped); }

void PlayerController::set_shot(bool flag) { key_map[ControllerInput::shoot] = flag ? 1.f : 0.f; }

float PlayerController::arms_switch() { return key_map[ControllerInput::arms_switch]; }

std::optional<float> PlayerController::get_controller_state(ControllerInput key) const {
	if (auto search = key_map.find(key); search != key_map.end()) {
		return search->second;
	} else {
		return std::nullopt;
	}
}

bool PlayerController::moving() { return key_map[ControllerInput::move_x] != 0.f; }

bool PlayerController::moving_left() { return key_map[ControllerInput::move_x] < 0.f; }

bool PlayerController::moving_right() { return key_map[ControllerInput::move_x] > 0.f; }

bool PlayerController::facing_left() const { return direction.lr == dir::LR::left; }

bool PlayerController::facing_right() const { return direction.lr == dir::LR::right; }

bool PlayerController::restricted() const { return flags.test(MovementState::restricted); }

bool PlayerController::grounded() const { return flags.test(MovementState::grounded); }

bool PlayerController::jump_requested() const { return jump_request > -1; }

bool PlayerController::jump_released() const { return jump_flags.test(Jump::is_released) && jump_flags.test(Jump::jumping); }

bool PlayerController::can_jump() const { return jump_flags.test(Jump::can_jump); }

bool PlayerController::jumping() const { return jump_flags.test(Jump::jumping); }

bool PlayerController::just_jumped() const { return jump_flags.test(Jump::just_jumped); }

bool PlayerController::jump_held() const { return jump_flags.test(Jump::jump_held); }

bool PlayerController::shot() { return key_map[ControllerInput::shoot] == 1.f; }

bool PlayerController::inspecting() { return key_map[ControllerInput::inspect] == 1.f; }

bool PlayerController::jumpsquatting() const { return jump_flags.test(Jump::jumpsquatting); }

bool PlayerController::jumpsquat_trigger() const { return jump_flags.test(Jump::jumpsquat_trigger); }

int PlayerController::get_jump_request() const { return jump_request; }

} // namespace controllers
