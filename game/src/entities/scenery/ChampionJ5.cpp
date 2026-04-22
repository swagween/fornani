
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

ChampionJ5::ChampionJ5(automa::ServiceProvider& svc, world::Map& map) : Mobile{svc, "champion_j5_body", {80, 60}}, m_propeller{svc, "champion_j5_propeller", {80, 60}} {
	push_and_set_animation("flying", {0, 1, 24, -1});
	push_animation("land", {1, 4, 24, 0});
	push_animation("grounded", {5, 1, 24, -1});
	push_animation("take_off", {6, 2, 24, 0});
	m_propeller.push_and_set_animation("spinning", {0, 3, 12, -1});
	m_propeller.center();
	center();
	Mobile::register_collider(map, {40.f, 40.f});
}

void ChampionJ5::update(world::Map& map) {
	tick();
	m_propeller.tick();
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, is_state(ChampionJ5State::flying));
	get_collider().set_flag(shape::ColliderFlags::simple, !is_state(ChampionJ5State::grounded));
	if (get_collider().has_flag_set(shape::ColliderFlags::simple)) { m_steering.seek(get_collider().physics, m_target, 0.00045f); }
	if ((get_collider().physics.position - m_target).length() < 8.f) { request(ChampionJ5State::land); }
	state_function = state_function();
}

void ChampionJ5::render(sf::RenderWindow& win, sf::Vector2f cam) {
	m_propeller.set_position(get_collider().get_center() - cam);
	win.draw(m_propeller);
	set_position(get_collider().get_center() - cam);
	win.draw(*this);
}

fsm::StateFunction ChampionJ5::update_flying() {
	p_state.actual = ChampionJ5State::flying;
	if (change_state(ChampionJ5State::land, get_params("land"))) { return CHAMPIONJ5_BIND(update_land); }
	return CHAMPIONJ5_BIND(update_flying);
}

fsm::StateFunction ChampionJ5::update_land() {
	p_state.actual = ChampionJ5State::land;
	if (animation.is_complete()) {
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
	if (animation.is_complete()) {
		request(ChampionJ5State::flying);
		if (change_state(ChampionJ5State::flying, get_params("flying"))) { return CHAMPIONJ5_BIND(update_flying); }
	}
	return CHAMPIONJ5_BIND(update_take_off);
}

bool ChampionJ5::change_state(ChampionJ5State next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani
