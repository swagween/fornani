#include "PlayerController.hpp"
#include "../../service/ServiceProvider.hpp"

namespace player {

PlayerController::PlayerController(automa::ServiceProvider& svc) : shield(svc) {
	key_map.insert(std::make_pair(ControllerInput::move_x, 0.f));
	key_map.insert(std::make_pair(ControllerInput::jump, 0.f));
	key_map.insert(std::make_pair(ControllerInput::sprint, 0.f));
	key_map.insert(std::make_pair(ControllerInput::shield, 0.f));
	key_map.insert(std::make_pair(ControllerInput::shoot, 0.f));
	key_map.insert(std::make_pair(ControllerInput::arms_switch, 0.f));
	key_map.insert(std::make_pair(ControllerInput::inspect, 0.f));
	key_map.insert(std::make_pair(ControllerInput::dash, 0.f));
	direction.und = dir::UND::neutral;
	direction.lr = dir::LR::right;
}

void PlayerController::update(automa::ServiceProvider& svc) {
	if (walking_autonomously()) { return; }

	auto const& left = svc.controller_map.label_to_control.at("left").held();
	auto const& right = svc.controller_map.label_to_control.at("right").held();
	auto const& up = svc.controller_map.label_to_control.at("up").held();
	auto const& down = svc.controller_map.label_to_control.at("down").held();

	auto const& sprint = svc.controller_map.label_to_control.at("sprint").held();
	auto const& sprint_release = svc.controller_map.label_to_control.at("sprint").released();

	auto const& shielding = svc.controller_map.label_to_control.at("shield").held();
	auto const& shield_pressed = svc.controller_map.label_to_control.at("shield").triggered();
	auto const& shield_released = svc.controller_map.label_to_control.at("shield").released();

	auto const& jump_started = svc.controller_map.label_to_control.at("main_action").triggered();
	auto const& jump_held = svc.controller_map.label_to_control.at("main_action").held();
	auto const& jump_released = svc.controller_map.label_to_control.at("main_action").released();

	auto const& shoot_pressed = svc.controller_map.label_to_control.at("secondary_action").triggered();
	auto const& shoot_released = svc.controller_map.label_to_control.at("secondary_action").released();

	auto const& arms_switch_left = svc.controller_map.label_to_control.at("arms_switch_left").triggered();
	auto const& arms_switch_right = svc.controller_map.label_to_control.at("arms_switch_right").triggered();

	auto const& inspected = (svc.controller_map.label_to_control.at("inspect").triggered() && grounded() && !left && !right) || (svc.controller_map.label_to_control.at("down").triggered() && grounded() && !left && !right);
	auto const& dash_left = svc.controller_map.label_to_control.at("tertiary_action").triggered() && !grounded() && left;
	auto const& dash_right = svc.controller_map.label_to_control.at("tertiary_action").triggered() && !grounded() && right;

	auto const& transponder_skip = svc.controller_map.label_to_control.at("main_action").triggered();
	auto const& transponder_skip_released = svc.controller_map.label_to_control.at("main_action").released();
	auto const& transponder_next = svc.controller_map.label_to_control.at("main_action").triggered();
	auto const& transponder_exit = svc.controller_map.label_to_control.at("secondary_action").triggered();
	auto const& transponder_down = svc.controller_map.label_to_control.at("down").triggered();
	auto const& transponder_up = svc.controller_map.label_to_control.at("up").triggered();
	auto const& transponder_left = svc.controller_map.label_to_control.at("left").triggered();
	auto const& transponder_right = svc.controller_map.label_to_control.at("right").triggered();
	auto const& transponder_select = svc.controller_map.label_to_control.at("main_action").triggered();

	auto const& hook_held = svc.controller_map.label_to_control.at("secondary_action").held();

	horizontal_inputs.push_back(key_map[ControllerInput::move_x]);
	if (horizontal_inputs.size() > quick_turn_sample_size) { horizontal_inputs.pop_front(); }

	key_map[ControllerInput::move_x] = svc.controller_map.get_throttle().x;
	key_map[ControllerInput::move_y] = svc.controller_map.get_throttle().y;
	key_map[ControllerInput::move_x] = left && !right ? -1.f : key_map[ControllerInput::move_x];
	key_map[ControllerInput::move_x] = right && !left ? 1.f : key_map[ControllerInput::move_x];
	key_map[ControllerInput::move_x] = right && left ? 0.f : key_map[ControllerInput::move_x];
	key_map[ControllerInput::move_y] = up && !down ? -1.f : key_map[ControllerInput::move_y];
	key_map[ControllerInput::move_y] = down && !up ? 1.f : key_map[ControllerInput::move_y];
	key_map[ControllerInput::move_y] = right && left ? 0.f : key_map[ControllerInput::move_y];

	// shield
	key_map[ControllerInput::shield] = 0.f;
	if (!shield.recovering() && grounded()) {
		if (shielding) { key_map[ControllerInput::shield] = 1.0f; }
		if (shielding) { shield.flags.triggers.set(ShieldTrigger::shield_up); }
		if (shield_released && shield.is_shielding()) { shield.pop(); }
	}

	key_map[ControllerInput::sprint] = 0.f;
	if (moving() && sprint && !sprint_released()) { key_map[ControllerInput::sprint] = key_map[ControllerInput::move_x]; }

	direction.set_intermediate(left, right, up, down);

	key_map[ControllerInput::dash] = dash_left && !dash_right ? -1.f : key_map[ControllerInput::dash];
	key_map[ControllerInput::dash] = dash_right && !dash_left ? 1.f : key_map[ControllerInput::dash];
	if (key_map[ControllerInput::dash] != 0.f && dash_count == 0) { dash_request = dash_time; }

	// transponder flags
	transponder_skip ? transponder_flags.set(TransponderInput::skip) : transponder_flags.reset(TransponderInput::skip);
	transponder_skip_released ? transponder_flags.set(TransponderInput::skip_released) : transponder_flags.reset(TransponderInput::skip_released);
	transponder_next ? transponder_flags.set(TransponderInput::next) : transponder_flags.reset(TransponderInput::next);
	transponder_exit ? transponder_flags.set(TransponderInput::exit) : transponder_flags.reset(TransponderInput::exit);
	transponder_down ? transponder_flags.set(TransponderInput::down) : transponder_flags.reset(TransponderInput::down);
	transponder_up ? transponder_flags.set(TransponderInput::up) : transponder_flags.reset(TransponderInput::up);
	transponder_left ? transponder_flags.set(TransponderInput::left) : transponder_flags.reset(TransponderInput::left);
	transponder_right ? transponder_flags.set(TransponderInput::right) : transponder_flags.reset(TransponderInput::right);
	transponder_select ? transponder_flags.set(TransponderInput::select) : transponder_flags.reset(TransponderInput::select);

	// hook
	hook_held ? hook_flags.set(Hook::hook_held) : hook_flags.reset(Hook::hook_held);

	// sprint
	if (sprint_release) { sprint_flags.set(Sprint::released); }
	if (grounded()) { sprint_flags = {}; }

	key_map[ControllerInput::jump] = jump_started ? 1.f : 0.f;

	if (shoot_pressed) { key_map[ControllerInput::shoot] = 1.f; }
	if (shoot_released) {
		key_map[ControllerInput::shoot] = 0.f;
		hook_flags.set(Hook::hook_released);
	}

	if (!restricted() && !shot()) {
		direction.lr = moving_left() ? dir::LR::left : direction.lr;
		direction.lr = moving_right() ? dir::LR::right : direction.lr;
		direction.und = dir::UND::neutral;
		direction.und = up ? dir::UND::up : direction.und;
		direction.und = down ? dir::UND::down : direction.und;
	} else if ((moving_left() && direction.lr == dir::LR::right) || (moving_right() && direction.lr == dir::LR::left)) {
		key_map[ControllerInput::move_x] *= backwards_dampen;
	}

	key_map[ControllerInput::arms_switch] = 0.f;
	key_map[ControllerInput::arms_switch] = arms_switch_left ? -1.f : key_map[ControllerInput::arms_switch];
	key_map[ControllerInput::arms_switch] = arms_switch_right ? 1.f : key_map[ControllerInput::arms_switch];

	key_map[ControllerInput::inspect] = inspected ? 1.f : 0.f;

	bool can_launch = !restricted() && flags.test(MovementState::grounded) && !jump.launched();
	can_launch ? jump.states.set(JumpState::can_jump) : jump.states.reset(JumpState::can_jump);

	if (jump_started) { jump.request_jump(); }
	if (jump_held) {
		jump.states.set(JumpState::jump_held);
	} else {
		jump.states.reset(JumpState::jump_held);
	}
	if (jump_released) { jump.triggers.set(JumpTrigger::is_released); }

	if (jump.requested() && flags.test(MovementState::grounded)) {
		jump.triggers.set(JumpTrigger::jumpsquat);
		jump.prevent();
	}

	flags.reset(MovementState::restricted);
	decrement_requests();
	jump.update();

	svc.controller_map.reset_triggers();
}

void PlayerController::clean() {
	flags = {};
	hook_flags = {};
}


void PlayerController::stop() {
	key_map[ControllerInput::move_x] = 0.f;
	key_map[ControllerInput::jump] = 0.f;
}

void PlayerController::ground() { flags.set(MovementState::grounded); }

void PlayerController::unground() { flags.reset(MovementState::grounded); }

void PlayerController::restrict() { flags.set(MovementState::restricted); }

void PlayerController::unrestrict() { flags.reset(MovementState::restricted); }

void PlayerController::uninspect() { key_map[ControllerInput::inspect] = 0.f; }

void PlayerController::stop_dashing() { key_map[ControllerInput::dash] = 0.f; }



void PlayerController::decrement_requests() {
	--dash_request;
	if (dash_request < 0) { dash_request = -1; }
}

void PlayerController::reset_dash_count() { dash_count = 0; }

void PlayerController::cancel_dash_request() { dash_request = -1; }

void PlayerController::dash() { dash_count = 1; }



void PlayerController::autonomous_walk() {
	direction.lr == dir::LR::right ? key_map[ControllerInput::move_x] = 1.f : key_map[ControllerInput::move_x] = -1.f;
	flags.set(MovementState::walking_autonomously);
}

void PlayerController::stop_walking_autonomously() { flags.reset(MovementState::walking_autonomously); }

void PlayerController::set_shot(bool flag) { key_map[ControllerInput::shoot] = flag ? 1.f : 0.f; }

void PlayerController::prevent_movement() {
	key_map[ControllerInput::move_x] = 0.f;
	key_map[ControllerInput::move_y] = 0.f;
	key_map[ControllerInput::dash] = 0.f;
	key_map[ControllerInput::arms_switch] = 0.f;
	key_map[ControllerInput::inspect] = 0.f;
	key_map[ControllerInput::shoot] = 0.f;
	key_map[ControllerInput::jump] = 0.f;
	jump.reset_all();
	jump.prevent();
	flags.set(MovementState::restricted);
}

void PlayerController::release_hook() { hook_flags.reset(Hook::hook_released); }

void PlayerController::nullify_dash() {
	cancel_dash_request();
	stop_dashing();
}

std::optional<float> PlayerController::get_controller_state(ControllerInput key) const {
	if (auto search = key_map.find(key); search != key_map.end()) {
		return search->second;
	} else {
		return std::nullopt;
	}
}


} // namespace controllers
