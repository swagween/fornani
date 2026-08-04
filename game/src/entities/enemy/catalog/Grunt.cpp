
#include <fornani/entities/enemy/catalog/Grunt.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Grunt::Grunt(automa::ServiceProvider& svc, world::Map& map, int variant) : Enemy(svc, map, "grunt"), m_services{&svc}, m_variant{static_cast<GruntVariant>(variant)} {
	p_animatable.set_animations({{"idle", {0, 2, 100, -1}}, {"drink", {2, 2, 32, 4}}});
	p_animatable.animation.set_params(get_params("idle"));
	p_state.actual = GruntState::idle;

	flags.state.set(StateFlags::vulnerable);
	p_animatable.random_frame_start();
}

void Grunt::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	if (svc.ticker.every_second() && random::percent_chance(5)) { request(GruntState::drink); }

	state_function = state_function();
}

void Grunt::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction Grunt::update_idle() {
	p_state.actual = GruntState::idle;
	if (change_state(GruntState::drink, get_params("drink"))) { return GRUNT_BIND(update_drink); }
	return GRUNT_BIND(update_idle);
}

fsm::StateFunction Grunt::update_drink() {
	p_state.actual = GruntState::drink;
	if (p_animatable.animation.just_started()) {
		if (!m_services->state_flags.test(automa::StateFlags::cutscene)) { m_services->soundboard.play_sound("tank_sip", get_collider().get_center()); }
	}
	if (p_animatable.animation.complete()) {
		request(GruntState::idle);
		if (change_state(GruntState::idle, get_params("idle"))) { return GRUNT_BIND(update_idle); }
	}
	return GRUNT_BIND(update_drink);
}

bool Grunt::change_state(GruntState next, anim::Parameters params) {
	if (p_state.desired == next) {
		p_animatable.animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
