
#include <fornani/entities/enemy/catalog/Crow.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto crow_framerate = 6;

Crow::Crow(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f spread) : Enemy(svc, map, "crow"), m_services{&svc}, m_evade_force{15.f}, m_home_force{0.01f}, m_start{spread}, m_fear{80}, m_init{4} {
	m_params = {{"idle", {0, 1, crow_framerate * 2, -1}}, {"peck", {4, 1, crow_framerate * 4, 0}}, {"hop", {5, 1, crow_framerate * 8, 0}}, {"turn", {1, 1, crow_framerate * 2, 0}}, {"fly", {2, 2, crow_framerate * 2, -1}}};
	animation.set_params(get_params("idle"));
	m_init.start();
	if (random::percent_chance(0.1f)) {
		m_variant = CrowVariant::mythic;
		attributes.loot_multiplier = 300.f;
		attributes.rare_drop_id = 0;
		attributes.gem_multiplier = 20.f;
		attributes.drop_range = {8, 10};
		flags.general.set(GeneralFlags::rare_drops);
		flags.general.set(GeneralFlags::custom_channels);
		m_custom_channel = EnemyChannel::invincible;
	}
}

void Crow::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	m_init.update();
	if (m_init.is_almost_complete()) { get_collider().physics.position += m_start; }
	if (m_init.running()) { return; }
	Enemy::update(svc, map, player);
	flags.state.set(StateFlags::vulnerable);
	m_fear.update();

	if (!map.within_bounds(get_collider().get_center())) { despawn(); }

	get_collider().set_flag(shape::ColliderFlags::simple, is_state(CrowState::fly));
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, is_state(CrowState::fly));
	get_collider().set_attribute(shape::ColliderAttributes::no_collision, is_state(CrowState::fly));
	is_state(CrowState::fly) ? get_collider().physics.set_friction_componentwise({0.999f, 0.999f}) : get_collider().physics.set_friction_componentwise({0.95f, 0.999f});

	m_evade_force = m_flee_direction.as_float() * m_random_x;

	// caution
	if (is_state(CrowState::fly)) {
		get_collider().physics.velocity.x = m_evade_force;
		m_steering.evade(get_collider().physics, player.get_collider().get_center(), m_evade_force);
	} else {
		m_flee_direction = player.get_actual_direction();
	}

	if (svc.ticker.every_second()) {
		if (random::percent_chance(20)) { random::percent_chance(50) ? request(CrowState::hop) : request(CrowState::peck); }
	}
	if (is_alert() && !is_hostile()) { request(CrowState::turn); }
	if (is_hostile()) { m_fear.randomize(); }

	// check nearby crows
	for (auto const& other : map.enemy_catalog.get_enemies<Crow>()) {
		if (other->get_alert_range().overlaps(get_collider().bounding_box) && other->is_state(CrowState::fly) && !m_fear.running()) { m_fear.randomize(); }
	}

	if (m_fear.is_almost_complete()) { request(CrowState::fly); }

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	state_function = state_function();
}

void Crow::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

void Crow::gui_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { /*debug();*/ }

fsm::StateFunction Crow::update_idle() {
	p_state.actual = CrowState::idle;
	if (change_state(CrowState::turn, get_params("turn"))) { return CROW_BIND(update_turn); }
	if (change_state(CrowState::peck, get_params("peck"))) { return CROW_BIND(update_peck); }
	if (change_state(CrowState::hop, get_params("hop"))) { return CROW_BIND(update_hop); }
	if (change_state(CrowState::fly, get_params("fly"))) { return CROW_BIND(update_fly); }
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

fsm::StateFunction Crow::update_hop() {
	p_state.actual = CrowState::hop;
	if (animation.just_started()) {
		set_direction(random::percent_chance(50) ? SimpleDirection{LR::left} : SimpleDirection{LR::right});
		get_collider().physics.apply_force(sf::Vector2f{directions.actual.as_float() * 4.f, -2.f} * 20.f);
	}
	if (animation.is_complete()) {
		if (change_state(CrowState::fly, get_params("fly")) && get_collider().grounded()) { return CROW_BIND(update_fly); }
		if (change_state(CrowState::turn, get_params("turn"))) { return CROW_BIND(update_turn); }
		request(CrowState::idle);
		if (change_state(CrowState::idle, get_params("idle"))) { return CROW_BIND(update_idle); }
	}
	return CROW_BIND(update_hop);
}

fsm::StateFunction Crow::update_fly() {
	p_state.actual = CrowState::fly;
	set_direction(m_flee_direction);
	if (animation.just_started()) {
		m_random_x = random::random_range_float(0.2f, 0.3f);
		random::percent_chance(50) ? m_services->soundboard.flags.crow.set(audio::Crow::fly) : m_services->soundboard.flags.crow.set(audio::Crow::flap);
	}
	get_collider().physics.apply_force({0.f, -m_home_force});
	return CROW_BIND(update_fly);
}

fsm::StateFunction Crow::update_turn() {
	p_state.actual = CrowState::turn;
	set_direction(m_flee_direction);
	if (change_state(CrowState::fly, get_params("fly"))) { return CROW_BIND(update_fly); }
	if (change_state(CrowState::idle, get_params("idle"))) { return CROW_BIND(update_idle); }
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
		ImGui::SliderFloat("Home Force", &m_home_force, 0.1f, 4.0f, "%.2f");
		ImGui::SliderFloat("Evade Force", &m_evade_force, 0.5f, 20.0f);
		ImGui::SeparatorText("Info");
		ImGui::SeparatorText("Controls");
		ImGui::End();
	}
}

} // namespace fornani::enemy
