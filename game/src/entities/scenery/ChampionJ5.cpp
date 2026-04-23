
#include <fornani/entities/scenery/ChampionJ5.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

ChampionJ5::ChampionJ5(automa::ServiceProvider& svc, world::Map& map) : Mobile{svc, "champion_j5_body", {80, 60}}, m_propeller{svc, "champion_j5_propeller", {80, 60}}, m_thrust{0.017f, .118f, .991f, 260.f} {
	push_and_set_animation("flying", {0, 1, 24, -1});
	push_animation("land", {1, 4, 24, 0});
	push_animation("grounded", {5, 1, 24, -1});
	push_animation("take_off", {6, 2, 24, 0});
	m_propeller.push_and_set_animation("spinning", {0, 3, 12, -1});
	m_propeller.center();
	center();
	Mobile::register_collider(map, {40.f, 40.f});
	get_collider().physics.set_friction_componentwise({1.f, 1.f});
	get_collider().stats.GRAV = 4.2f;
	get_collider().set_trait(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::pushable);
	get_collider().set_exclusion_target(shape::ColliderTrait::block);
}

void ChampionJ5::update(world::Map& map) {
	tick();
	m_propeller.tick();
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_flag(shape::ColliderFlags::simple);

	/*if (flags.test(ChampionJ5Flags::interactable)) {
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, is_state(ChampionJ5State::flying));
		get_collider().set_flag(shape::ColliderFlags::simple, !is_state(ChampionJ5State::grounded));
	} else {
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
		get_collider().set_flag(shape::ColliderFlags::simple);
	}*/

	// get_collider().has_flag_set(shape::ColliderFlags::simple) ? get_collider().physics.set_friction_componentwise({0.9f, 0.9f}) : get_collider().physics.set_friction_componentwise({0.99f, 0.99f});

	// if (get_collider().has_flag_set(shape::ColliderFlags::simple)) { m_steering.seek(get_collider().physics, m_target, 0.00003f); }
	if (get_collider().has_flag_set(shape::ColliderFlags::simple)) { m_steering.thrust_seek(get_collider().physics, m_target, m_thrust); }
	// if ((get_collider().physics.position - m_target).length() < 8.f && flags.test(ChampionJ5Flags::interactable)) { request(ChampionJ5State::land); }

	state_function = state_function();
}

void ChampionJ5::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto drawpos = get_collider().get_center() + sf::Vector2f{0.f, 4.f};
	m_propeller.set_position(drawpos - cam);
	win.draw(m_propeller);
	set_position(drawpos - cam);
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
