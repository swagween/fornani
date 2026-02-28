
#include <fornani/entities/enemy/catalog/Mizzle.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto mizzle_framerate = 6;

Mizzle::Mizzle(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "mizzle"), m_services{&svc} {
	m_params = {{"idle", {0, 4, mizzle_framerate * 2, -1}}, {"turn", {4, 1, mizzle_framerate * 2, 0}}};
	animation.set_params(get_params("idle"));
	p_state.actual = MizzleState::idle;

	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);

	get_collider().physics.set_friction_componentwise({0.995f, 0.99f});
}

void Mizzle::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	face_player(player);
	flags.state.set(StateFlags::vulnerable);

	if (is_alert()) { m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{0.f, -8.f}, 0.00006f); }

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	if (directions.actual.lnr != directions.desired.lnr) { request(MizzleState::turn); }

	state_function = state_function();
}

void Mizzle::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Enemy::render(svc, win, cam);
	if (health.is_dead()) { return; }
}

fsm::StateFunction Mizzle::update_idle() {
	p_state.actual = MizzleState::idle;
	if (change_state(MizzleState::turn, get_params("turn"))) { return MIZZLE_BIND(update_turn); }
	return MIZZLE_BIND(update_idle);
}
fsm::StateFunction Mizzle::update_turn() {
	p_state.actual = MizzleState::turn;
	if (animation.complete()) {
		request_flip();
		request(MizzleState::idle);
		if (change_state(MizzleState::idle, get_params("idle"))) { return MIZZLE_BIND(update_idle); }
	}
	return MIZZLE_BIND(update_turn);
}

bool Mizzle::change_state(MizzleState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
