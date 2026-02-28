
#include <fornani/entities/enemy/catalog/MizzleEgg.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

constexpr auto mizzle_egg_framerate = 32;

MizzleEgg::MizzleEgg(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, map, "mizzle_egg"), m_services{&svc}, m_map{&map}, m_hatch_timer{400}, m_mizzle_spawn{600} {
	m_params = {{"closed", {0, 1, mizzle_egg_framerate, -1}}, {"in_between", {1, 1, mizzle_egg_framerate, 0}}, {"open", {2, 1, mizzle_egg_framerate, -1}}, {"hatch", {3, 1, mizzle_egg_framerate, -1}}};
	animation.set_params(get_params("closed"));
	p_state.actual = MizzleEggState::closed;
	set_root(map);
	flags.general.reset(GeneralFlags::gravity);
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	flags.state.set(StateFlags::vulnerable);
	init.start();
	m_hatch_timer.start();
	if (random::coin_flip()) { request_flip(); }
}

void MizzleEgg::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	init.update();
	if (init.is_almost_complete()) { set_root(map); }
	if (init.running()) { return; }
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	Enemy::update(svc, map, player);

	if (!m_flags.test(MizzleEggFlags::detached)) {
		m_steering.seek(get_collider().physics, m_root->get_bob(), 0.001f);
		get_collider().physics.set_global_friction(0.999f);
		if (m_hatch_timer.halfway()) { shake(); }
	}

	if (m_hatch_timer.is_almost_complete()) {
		flags.general.set(GeneralFlags::gravity);
		Enemy::get_collider().set_flag(shape::ColliderFlags::simple, false);
		m_flags.set(MizzleEggFlags::detached);
	}

	is_alert() ? request(MizzleEggState::open) : request(MizzleEggState::closed);
	if (is_hostile()) { request(MizzleEggState::open); }
	if (m_flags.test(MizzleEggFlags::detached) && get_collider().grounded() && !m_flags.test(MizzleEggFlags::hatched)) {
		request(MizzleEggState::hatch);
		spawn_mizzle();
		m_flags.set(MizzleEggFlags::hatched);
	}

	// hurt
	if (flags.state.test(StateFlags::hurt)) {
		if (!hurt_effect.running()) { hurt_effect.start(128); }
		flags.state.reset(StateFlags::hurt);
	}

	state_function = state_function();
}

void MizzleEgg::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) { Enemy::render(svc, win, cam); }

fsm::StateFunction MizzleEgg::update_closed() {
	p_state.actual = MizzleEggState::closed;
	m_hatch_timer.reverse();
	if (change_state(MizzleEggState::hatch, get_params("hatch"))) { return MIZZLE_EGG_BIND(update_hatch); }
	if (change_state(MizzleEggState::open, get_params("in_between"))) { return MIZZLE_EGG_BIND(update_in_between); }
	return MIZZLE_EGG_BIND(update_closed);
}

fsm::StateFunction MizzleEgg::update_in_between() {
	p_state.actual = MizzleEggState::in_between;
	if (change_state(MizzleEggState::hatch, get_params("hatch"))) { return MIZZLE_EGG_BIND(update_hatch); }
	if (animation.is_complete()) {
		if (change_state(MizzleEggState::closed, get_params("closed"))) { return MIZZLE_EGG_BIND(update_closed); }
		if (change_state(MizzleEggState::open, get_params("open"))) { return MIZZLE_EGG_BIND(update_open); }
	}
	return MIZZLE_EGG_BIND(update_in_between);
}

fsm::StateFunction MizzleEgg::update_open() {
	p_state.actual = MizzleEggState::open;
	m_hatch_timer.update();
	if (change_state(MizzleEggState::hatch, get_params("hatch"))) { return MIZZLE_EGG_BIND(update_hatch); }
	if (change_state(MizzleEggState::closed, get_params("in_between"))) { return MIZZLE_EGG_BIND(update_in_between); }
	return MIZZLE_EGG_BIND(update_open);
}

fsm::StateFunction MizzleEgg::update_hatch() {
	p_state.actual = MizzleEggState::hatch;
	m_mizzle_spawn.update();
	if (m_mizzle_spawn.is_almost_complete()) { spawn_mizzle(); }
	return MIZZLE_EGG_BIND(update_hatch);
}

void MizzleEgg::set_root(world::Map& map) {
	auto closest = std::numeric_limits<float>::max();
	for (auto& vine : map.get_entities<Vine>()) {
		for (auto& link : vine->get_chain().links) {
			auto distance = (get_collider().get_center() - link.get_bob()).length();
			if (distance < closest) {
				m_root = &link;
				closest = distance;
			}
		}
	}
}

void MizzleEgg::spawn_mizzle() {
	m_map->spawn_enemy(26, get_collider().get_center(), 0, true);
	m_mizzle_spawn.start();
	m_map->spawn_effect(*m_services, "demon_breath", get_collider().get_center());
}

bool MizzleEgg::change_state(MizzleEggState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
