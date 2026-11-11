
#include <fornani/entities/enemy/boss/DogfightNimbus.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

DogfightNimbus::DogfightNimbus(automa::ServiceProvider& svc, world::Map& map)
	: Boss(svc, map, "dogfight_nimbus"), m_turret{svc, "aerial_missile"}, m_pursuit_parameters{.trailing_distance = 20.f, .prediction_time = 0.9f, .stiffness = 0.0001f, .damping = 0.0019f, .max_force = 0.05f, .wander_strength = 0.0025f},
	  m_recharge{1000}, m_fire{90}, m_rounds{2}, m_barrel_roll{360} {
	p_animatable.set_animations({{"idle", {0, 8, 40, -1}}, {"turn_down", {8, 4, 40, -1}}, {"turn_up", {12, 4, 40, -1}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = DogfightNimbusState::idle;
	flags.state.set(StateFlags::vulnerable);
	attributes.team = arms::Team::skycorps;
	m_turret.set_team(arms::Team::skycorps);
	set_direction({LR::right});
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	get_collider().physics.set_friction_componentwise({0.992f, 0.999f});

	m_rounds.start();
	m_recharge.randomize();
	set_flag(BossFlags::ignore_oob);

	start_battle();
}

void DogfightNimbus::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Boss::update(svc, map, player);
	if (died()) { return; }

	m_recharge.update();
	m_fire.update();
	m_barrel_roll.update();

	// offset adjustment logic
	auto const center_x = p_reference_point.x + 480.f;
	if (player.get_collider().get_center().x > center_x + m_flip_threshold && m_current_side.right()) { m_current_side.flip(); }
	if (player.get_collider().get_center().x < center_x - m_flip_threshold && m_current_side.left()) { m_current_side.flip(); }
	m_offset.x = m_current_side.left() ? -280.f : 500.f;
	if (svc.ticker.every_x_ticks(2000)) { m_offset.y *= -1.f; }

	// gun
	m_turret.get().set_firing_direction(CardinalDirection{UDLR::right});
	m_turret.update(svc, map, *this);
	if (m_recharge.is_complete()) {
		if (m_fire.is_complete()) {
			auto bp = m_rounds.get() == 1 ? sf::Vector2f{0.f, 30.f} : sf::Vector2f{60.f, 20.f};
			m_turret.get().set_barrel_point(get_collider().get_center() + bp);
			m_turret.shoot(svc, map, player.get_collider().get_center() - m_turret.barrel_point());
			m_fire.start();
			m_rounds.update();
		}
		if (m_rounds.is_complete()) {
			m_rounds.start();
			m_recharge.start();
		}
	}

	// caution
	auto const evade_speed = 1.5f;
	if (auto const iproj = m_caution.incoming_projectile(map, physical.alert_range, arms::Team::skycorps)) {
		if (!m_barrel_roll.running()) {
			auto const above = iproj->y < get_collider().get_center().y;
			get_collider().physics.velocity.y = above ? evade_speed : -evade_speed;
			m_barrel_roll.start();
		}
	}

	// pursuit
	auto const velocity = player.get_collider().physics.velocity;
	auto const speed = std::hypot(velocity.x, velocity.y);
	if (speed > 0.001f) { m_rotation = std::atan2(velocity.y, velocity.x) * util::rad_to_deg; }
	auto const radians = m_rotation * util::deg_to_rad;
	auto const forward = sf::Vector2f{std::cos(radians), std::sin(radians)};
	auto const pposition = player.is_dead() ? sf::Vector2f{player.get_position().x, map.real_dimensions.y * 0.5f} : player.get_position() + m_offset;
	m_steering.pursue(get_collider().physics, pposition, velocity, forward, m_pursuit_parameters);

	// effects
	if (svc.ticker.every_x_ticks(30)) { map.spawn_effect(svc, "exhaust_kestrel", get_collider().get_center() + sf::Vector2f{-104.f, -38.f}); }

	state_function = state_function();
}

void DogfightNimbus::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Boss::render(svc, win, cam);
	// debug();
}

void DogfightNimbus::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Boss::gui_render(svc, win, cam); }

fsm::StateFunction DogfightNimbus::update_idle() {
	p_state.actual = DogfightNimbusState::idle;
	if (get_collider().physics.actual_velocity().y < -m_turn_factor) {
		request(DogfightNimbusState::turn_up);
		if (change_state(DogfightNimbusState::turn_up, get_params("turn_up"))) { return DOGFIGHT_NIMBUS_BIND(update_turn_up); }
	}
	if (get_collider().physics.actual_velocity().y > m_turn_factor) {
		request(DogfightNimbusState::turn_down);
		if (change_state(DogfightNimbusState::turn_down, get_params("turn_down"))) { return DOGFIGHT_NIMBUS_BIND(update_turn_down); }
	}
	return DOGFIGHT_NIMBUS_BIND(update_idle);
}

fsm::StateFunction DogfightNimbus::update_turn_down() {
	p_state.actual = DogfightNimbusState::turn_down;
	if (get_collider().physics.actual_velocity().y < m_turn_factor) {
		request(DogfightNimbusState::idle);
		if (change_state(DogfightNimbusState::idle, get_params("idle"))) { return DOGFIGHT_NIMBUS_BIND(update_idle); }
	}
	return DOGFIGHT_NIMBUS_BIND(update_turn_down);
}

fsm::StateFunction DogfightNimbus::update_turn_up() {
	p_state.actual = DogfightNimbusState::turn_up;
	if (get_collider().physics.actual_velocity().y > -m_turn_factor) {
		request(DogfightNimbusState::idle);
		if (change_state(DogfightNimbusState::idle, get_params("idle"))) { return DOGFIGHT_NIMBUS_BIND(update_idle); }
	}
	return DOGFIGHT_NIMBUS_BIND(update_turn_up);
}

bool DogfightNimbus::change_state(DogfightNimbusState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

void DogfightNimbus::debug() {
	auto const& physics = get_collider().physics;
	auto const position = get_collider().get_center();
	auto const velocity = physics.velocity;

	ImGui::Begin("Dogfight Nimbus Debug");

	ImGui::Text("Died: %s", died() ? "YES" : "NO");
	ImGui::SeparatorText("Position");

	ImGui::Text("Center:  X %.2f | Y %.2f", position.x, position.y);
	ImGui::Text("Velocity: X %.4f | Y %.4f", velocity.x, velocity.y);
	ImGui::Text("Speed: %.4f", std::hypot(velocity.x, velocity.y));

	ImGui::SeparatorText("Physics");

	auto const actual_velocity = physics.actual_velocity();
	ImGui::Text("Actual velocity: X %.4f | Y %.4f", actual_velocity.x, actual_velocity.y);

	ImGui::SeparatorText("Pursuit Tuning");
	ImGui::SliderFloat("Trailing Distance", &m_pursuit_parameters.trailing_distance, 20.f, 250.f, "%.1f");
	ImGui::SliderFloat("Prediction Time", &m_pursuit_parameters.prediction_time, 0.f, 1.f, "%.3f");
	ImGui::SliderFloat("Stiffness", &m_pursuit_parameters.stiffness, 0.0001f, 0.008f, "%.5f");
	ImGui::SliderFloat("Damping", &m_pursuit_parameters.damping, 0.0005f, 0.02f, "%.5f");
	ImGui::SliderFloat("Max Force", &m_pursuit_parameters.max_force, 0.001f, 0.06f, "%.4f");
	ImGui::SliderFloat("Wander Strength", &m_pursuit_parameters.wander_strength, 0.f, 0.005f, "%.5f");
	ImGui::SliderFloat("Wander Radius", &m_pursuit_parameters.wander_radius, 0.f, 200.f, "%.1f");
	if (ImGui::Button("Reset Pursuit")) { m_pursuit_parameters = {.trailing_distance = 20.f, .prediction_time = 0.9f, .stiffness = 0.0001f, .damping = 0.0019f, .max_force = 0.05f, .wander_strength = 0.0025f}; }

	ImGui::End();
}

} // namespace fornani::enemy
