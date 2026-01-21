
#include <fornani/entities/enemy/catalog/Crow.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto crow_framerate = 6;

Crow::Crow(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f spread) : Enemy(svc, map, "crow"), m_services{&svc}, m_evade_force{3.8f}, m_home_force{0.0001f}, m_friction{0.98f, 0.98f} {
	m_params = {{"idle", {0, 1, crow_framerate * 2, -1}}, {"peck", {4, 1, crow_framerate * 2, 0}}, {"turn", {1, 1, crow_framerate * 2, 0}}, {"fly", {2, 2, crow_framerate * 2, -1}}};
	animation.set_params(get_params("idle"));
	flags.state.set(StateFlags::no_shake);

	get_collider().physics.set_friction_componentwise({0.99f, 0.99f});
	set_position(get_collider().physics.position + random::random_vector_float({-spread.x, spread.x}, {}));
}

void Crow::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	Enemy::update(svc, map, player);
	flags.state.set(StateFlags::vulnerable);

	m_home = map.get_closest_home_point(get_collider().physics.position);
	get_collider().set_flag(shape::ColliderFlags::simple, is_state(CrowState::fly));
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, is_state(CrowState::fly));
	get_collider().set_attribute(shape::ColliderAttributes::no_collision, is_state(CrowState::fly));
	get_collider().physics.gravity = is_state(CrowState::fly) ? -40.f : 4.f;
	is_state(CrowState::fly) ? get_collider().physics.set_friction_componentwise(m_friction) : get_collider().physics.set_friction_componentwise(m_friction * 0.9f);
	// if ((get_collider().physics.position - m_home).length() < 30.f) { request(CrowState::idle); }

	// caution
	m_steering.evade(get_collider().physics, player.get_collider().get_center(), m_evade_force);
	m_steering.target(get_collider().physics, m_home, m_home_force);

	if (!get_collider().grounded() || is_alert()) {
		request(CrowState::fly);
	} else {
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	state_function = state_function();
}

void Crow::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

void Crow::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { debug(); }

fsm::StateFunction Crow::update_idle() {
	p_state.actual = CrowState::idle;
	if (change_state(CrowState::turn, get_params("turn"))) { return CROW_BIND(update_turn); }
	if (change_state(CrowState::peck, get_params("peck"))) { return CROW_BIND(update_peck); }
	if (change_state(CrowState::fly, get_params("fly")) && get_collider().grounded()) { return CROW_BIND(update_fly); }
	return CROW_BIND(update_idle);
}

fsm::StateFunction Crow::update_peck() {
	p_state.actual = CrowState::peck;
	if (animation.is_complete()) {
		if (change_state(CrowState::fly, get_params("fly")) && get_collider().grounded()) { return CROW_BIND(update_fly); }
		if (change_state(CrowState::turn, get_params("turn"))) { return CROW_BIND(update_turn); }
		request(CrowState::idle);
		if (change_state(CrowState::idle, get_params("idle"))) { return CROW_BIND(update_idle); }
	}
	return CROW_BIND(update_peck);
}

fsm::StateFunction Crow::update_fly() {
	p_state.actual = CrowState::fly;
	/*if (animation.is_complete()) {
		if (change_state(CrowState::idle, get_params("idle"))) { return CROW_BIND(update_idle); }
	}*/
	return CROW_BIND(update_fly);
}

fsm::StateFunction Crow::update_turn() {
	p_state.actual = CrowState::turn;
	if (animation.complete()) {
		if (change_state(CrowState::fly, get_params("fly")) && get_collider().grounded()) { return CROW_BIND(update_fly); }
		request(CrowState::idle);
		if (change_state(CrowState::idle, get_params("idle"))) { return CROW_BIND(update_idle); }
	}
	return CROW_BIND(update_turn);
}

bool Crow::change_state(CrowState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

void Crow::debug() {
	static auto sz = ImVec2{360.f, 250.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Crow Debug")) {
		ImGui::SeparatorText("Parameters");
		ImGui::SliderFloat("X Friction", &m_friction.x, 0.98f, 0.999f);
		ImGui::SliderFloat("Y Friction", &m_friction.y, 0.98f, 0.999f);
		ImGui::SliderFloat("Home Force", &m_home_force, 0.0f, 0.001f, "%.5f");
		ImGui::SliderFloat("Evade Force", &m_evade_force, 0.5f, 4.0f);
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		ImGui::End();
	}
}

} // namespace fornani::enemy
