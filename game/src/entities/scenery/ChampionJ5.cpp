
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

ChampionJ5::ChampionJ5(automa::ServiceProvider& svc, world::Map& map) : Mobile{svc, "champion_j5_body", {80, 60}}, m_services{&svc}, m_propeller{svc, "champion_j5_propeller", {80, 60}}, m_thrust{0.027f, .418f, .99f, 600.f} {
	p_animatable.push_and_set_animation("flying", {0, 1, 24, -1});
	p_animatable.push_animation("land", {1, 4, 24, 0});
	p_animatable.push_animation("grounded", {5, 1, 24, -1});
	p_animatable.push_animation("take_off", {6, 2, 24, 0});
	m_propeller.push_and_set_animation("spinning", {0, 3, 12, -1});
	m_propeller.push_animation("retract", {3, 8, 18, 0, true});
	m_propeller.center();
	p_animatable.center();
	Mobile::register_collider(map, {40.f, 40.f});
	get_collider().physics.set_friction_componentwise({1.f, 1.f});
	get_collider().stats.GRAV = 4.2f;
	get_collider().set_trait(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::chest);
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::pushable);
	get_collider().set_exclusion_target(shape::ColliderTrait::block);
	get_collider().set_exclusion_target(shape::ColliderTrait::particle);
}

void ChampionJ5::update(automa::ServiceProvider& svc, world::Map& map) {
	p_animatable.tick();
	m_propeller.tick();
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_flag(shape::ColliderFlags::simple);
	if (m_propeller.is_animation("spinning")) {
		auto max_vel = 8.f;
		auto t = std::clamp(get_collider().physics.velocity.lengthSquared() / max_vel, 0.f, 1.f);
		auto pitch = std::lerp(0.5f, 1.1f, get_collider().physics.velocity.lengthSquared());
		svc.soundboard.repeat_sound("champion_j5_propeller", 977, get_collider().get_center(), pitch);
	}

	if (flags.test(ChampionJ5Flags::interactable)) {
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, is_state(ChampionJ5State::flying));
		get_collider().set_flag(shape::ColliderFlags::simple, !is_state(ChampionJ5State::grounded));
	}
	if (get_collider().grounded() && !flags.test(ChampionJ5Flags::landed)) {
		flags.set(ChampionJ5Flags::landed);
		svc.soundboard.play_sound("thud");
	}

	if (!flags.test(ChampionJ5Flags::controlled)) {

		get_collider().has_flag_set(shape::ColliderFlags::simple) ? get_collider().physics.set_friction_componentwise({1.f, 1.f}) : get_collider().physics.set_friction_componentwise({0.9f, 0.9f});

		// if (get_collider().has_flag_set(shape::ColliderFlags::simple)) { m_steering.seek(get_collider().physics, m_target, 0.00003f); }
		if (get_collider().has_flag_set(shape::ColliderFlags::simple)) { m_steering.thrust_seek(get_collider().physics, m_target, m_thrust); }
		if (is_close_to_target(4.f) && flags.test(ChampionJ5Flags::interactable)) { request(ChampionJ5State::land); }
	}

	state_function = state_function();

	if (is_state(ChampionJ5State::flying)) {
		p_animatable.set_frame(0);
		auto const factor = 0.5f;
		if (get_collider().physics.velocity.y < -factor) { p_animatable.set_frame(8); }
		if (get_collider().physics.velocity.y > factor) { p_animatable.set_frame(9); }
	}
}

void ChampionJ5::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto drawpos = get_collider().get_center() + sf::Vector2f{0.f, -17.f};
	m_propeller.set_position(drawpos - cam);
	win.draw(m_propeller);
	p_animatable.set_position(drawpos - cam);
	win.draw(p_animatable);
	// debug();
}

fsm::StateFunction ChampionJ5::update_flying() {
	p_state.actual = ChampionJ5State::flying;
	if (change_state(ChampionJ5State::land, get_params("land"))) { return CHAMPIONJ5_BIND(update_land); }
	return CHAMPIONJ5_BIND(update_flying);
}

fsm::StateFunction ChampionJ5::update_land() {
	p_state.actual = ChampionJ5State::land;
	if (p_animatable.animation.just_started()) { m_propeller.set_animation("retract"); }
	if (p_animatable.animation.get_frame_count() == 2 && p_animatable.animation.keyframe_started()) { m_services->soundboard.play_sound("champion_j5_land"); }
	if (p_animatable.animation.is_complete()) {
		request(ChampionJ5State::grounded);
		if (change_state(ChampionJ5State::grounded, get_params("grounded"))) { return CHAMPIONJ5_BIND(update_grounded); }
	}
	return CHAMPIONJ5_BIND(update_land);
}

fsm::StateFunction ChampionJ5::update_grounded() {
	p_state.actual = ChampionJ5State::grounded;
	if (change_state(ChampionJ5State::take_off, get_params("take_off"))) { return CHAMPIONJ5_BIND(update_take_off); }
	return CHAMPIONJ5_BIND(update_grounded);
}

fsm::StateFunction ChampionJ5::update_take_off() {
	p_state.actual = ChampionJ5State::take_off;
	if (p_animatable.animation.is_complete()) {
		request(ChampionJ5State::flying);
		if (change_state(ChampionJ5State::flying, get_params("flying"))) { return CHAMPIONJ5_BIND(update_flying); }
	}
	return CHAMPIONJ5_BIND(update_take_off);
}

auto ChampionJ5::get_drivers_seat() const -> sf::Vector2f { return get_global_center() + sf::Vector2f{10.f, -32.f + get_y_offset()}; }

auto ChampionJ5::get_passengers_seat() const -> sf::Vector2f { return get_global_center() + sf::Vector2f{-8.f, -26.f + get_y_offset()}; }

bool ChampionJ5::change_state(ChampionJ5State next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

float ChampionJ5::get_y_offset() const { return p_animatable.get_frame() == 9 ? 4.f : p_animatable.get_frame() == 8 ? -4.f : 0.f; }

void ChampionJ5::debug() {
	if (ImGui::Begin("Champion J5 - Flight Debug")) {
		auto const& physics = get_collider().physics;

		auto const actual_velocity = physics.actual_velocity();
		auto const apparent_velocity = physics.apparent_velocity();
		auto const apparent_acceleration = physics.apparent_acceleration();

		ImGui::SeparatorText("State");

		ImGui::Text("Flying: %s", is_state(ChampionJ5State::flying) ? "YES" : "NO");
		ImGui::Text("Animation frame: %d", p_animatable.get_frame());

		ImGui::SeparatorText("Position");

		ImGui::Text("Position:          %.4f, %.4f", physics.position.x, physics.position.y);
		ImGui::Text("Previous position: %.4f, %.4f", physics.previous_position.x, physics.previous_position.y);
		ImGui::Text("Displacement:      %.4f, %.4f", physics.position.x - physics.previous_position.x, physics.position.y - physics.previous_position.y);

		ImGui::SeparatorText("Velocity");

		ImGui::Text("Actual velocity:   %.4f, %.4f", actual_velocity.x, actual_velocity.y);
		ImGui::Text("Apparent velocity: %.4f, %.4f", apparent_velocity.x, apparent_velocity.y);

		ImGui::Text("Actual speed:      %.4f", physics.actual_speed());

		ImGui::SeparatorText("Acceleration");

		ImGui::Text("Previous velocity: %.4f, %.4f", physics.previous_velocity.x, physics.previous_velocity.y);
		ImGui::Text("Apparent accel:    %.4f, %.4f", apparent_acceleration.x, apparent_acceleration.y);

		ImGui::SeparatorText("Animation");

		ImGui::Text("Y velocity: %.4f", actual_velocity.y);
		ImGui::Text("Threshold:  %.6f", constants::tiny_value);

		if (actual_velocity.y < -constants::tiny_value) {
			ImGui::TextColored(ImVec4(0.2f, 1.f, 0.2f, 1.f), "MOVING UP");
		} else if (actual_velocity.y > constants::tiny_value) {
			ImGui::TextColored(ImVec4(1.f, 0.5f, 0.2f, 1.f), "MOVING DOWN");
		} else {
			ImGui::TextDisabled("NEUTRAL");
		}
	}
	ImGui::End();
}

} // namespace fornani
