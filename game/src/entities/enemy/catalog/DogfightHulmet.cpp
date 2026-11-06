
#include <fornani/entities/enemy/catalog/DogfightHulmet.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

DogfightHulmet::DogfightHulmet(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, map, "dogfight_hulmet"), m_services{&svc}, m_map{&map}, m_turret{svc, "aerial_turret"},
	  m_pursuit_parameters{.trailing_distance = 80.f, .prediction_time = 0.2f, .stiffness = random::range_float(0.0015f, 0.0025f), .damping = random::range_float(0.007f, 0.01f), .max_force = 0.035f} {
	p_animatable.set_animations({{"idle", {0, 4, 40, -1}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = DogfightHulmetState::idle;
	flags.state.set(StateFlags::vulnerable);
	attributes.team = arms::Team::skycorps;
	m_turret.set_team(arms::Team::skycorps);
	set_direction({LR::right});
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	get_collider().physics.set_friction_componentwise({0.993f, 0.993f});
	m_init = true;

	m_offset = sf::Vector2f{random::range_float(-300.f, -220.f), random::range_float(-60.f, 60.f)};
}

void DogfightHulmet::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	if (m_init) {
		get_collider().physics.velocity.x = 0.f;
		m_init = false;
	}

	// spew
	auto bp = sf::Vector2f{0.f, -80.f};
	m_turret.get().set_barrel_point(get_collider().get_center() + bp);
	m_turret.get().set_firing_direction(CardinalDirection{UDLR::right});
	m_turret.update(svc, map, *this);
	// m_steering.arrive(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{-300.f, 0.f});
	// m_steering.target(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{-300.f, 0.f});

	auto const velocity = player.get_collider().physics.velocity;
	auto const speed = std::hypot(velocity.x, velocity.y);
	if (speed > 0.001f) { m_rotation = std::atan2(velocity.y, velocity.x) * util::rad_to_deg; }
	auto const radians = m_rotation * util::deg_to_rad;
	auto const forward = sf::Vector2f{std::cos(radians), std::sin(radians)};

	m_steering.pursue(get_collider().physics, player.get_position() + m_offset, velocity, forward, m_pursuit_parameters);

	state_function = state_function();
}

void DogfightHulmet::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	// debug();
}

fsm::StateFunction DogfightHulmet::update_idle() {
	p_state.actual = DogfightHulmetState::idle;
	return DOGFIGHT_HULMET_BIND(update_idle);
}

bool DogfightHulmet::change_state(DogfightHulmetState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

void DogfightHulmet::debug() {
	auto const& physics = get_collider().physics;
	auto const position = get_collider().get_center();
	auto const velocity = physics.velocity;

	ImGui::Begin("Dogfight Helmet Debug");

	ImGui::Text("Died: %s", died() ? "YES" : "NO");
	ImGui::SeparatorText("Position");

	ImGui::Text("Center:  X %.2f | Y %.2f", position.x, position.y);
	ImGui::Text("Velocity: X %.4f | Y %.4f", velocity.x, velocity.y);
	ImGui::Text("Speed: %.4f", std::hypot(velocity.x, velocity.y));

	ImGui::SeparatorText("Physics");

	auto const actual_velocity = physics.actual_velocity();
	ImGui::Text("Actual velocity: X %.4f | Y %.4f", actual_velocity.x, actual_velocity.y);

	ImGui::End();
}

} // namespace fornani::enemy
