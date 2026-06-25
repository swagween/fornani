
#include <fornani/entities/enemy/catalog/Macrophage.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Macrophage::Macrophage(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, map, "macrophage"), m_services(&svc), m_map(&map),
	  m_body{std::make_unique<vfx::Chain>(svc, svc.assets.get_texture("macrophage_wall"), vfx::SpringParameters{0.99f, 0.08f, 16.5f, 4.f}, get_collider().get_center(), 32, false, 16.5f, true)} {
	p_animations = {{"idle", {0, 4, 40, -1}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.99f, 0.99f});
	attributes.team = arms::Team::beast;
	m_grab.set_constant_radius(40.f);
	flags.state.set(StateFlags::vulnerable);
	m_grab.sensor.activate();
	m_body->set_free(true);
}

void Macrophage::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);
	if (died()) { return; }

	m_body->set_position(get_collider().get_position());
	m_body->update(svc, map, player);
	get_collider().physics.velocity += map.compute_mtv(get_collider().get_center()) * 0.2f;

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) { svc.soundboard.play_sound("hit_low"); }
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	if (is_alert()) {
		face_player(player);

		auto force = std::lerp(0.002f, 0.006f, 1.f - m_body->get_percentage_colliding());
		m_steering.seek(Enemy::get_collider().physics, player.get_collider().get_center() + sf::Vector2f{0.f, -8.f}, force);
		get_collider().physics.apply_force(m_body->get_recoil_force());
	} else {
		face_movement();
		m_steering.smooth_random_walk(Enemy::get_collider().physics, 0.005f, 64.f);
	}

	if (m_flags.test(MacrophageFlags::caught_player)) {
		player.set_position(get_collider().get_center() + sf::Vector2f{directions.actual.as_float() * 24.f, 0.f}, true);
		player.get_collider().physics.zero();
	}
	if (m_flags.consume(MacrophageFlags::released_player)) { player.accumulated_momentum.push_back({directions.actual.as_float() * 10.f, -0.6f}); }
	m_grab.set_position(get_collider().get_center());
	if (m_grab.sensor.within_bounds(player.hurtbox) && m_grab.sensor.active() && !m_flags.test(MacrophageFlags::caught_player)) {
		// player.stun(1.f);
		// m_flags.set(MacrophageFlags::caught_player);
	}

	state_function = state_function();
}

void Macrophage::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	m_body->render(svc, win, cam);
	Enemy::render(svc, win, cam);
	if (died()) { return; }
	if (svc.greyblock_mode() && m_grab.sensor.active()) { m_grab.render(win, cam); }
}

fsm::StateFunction Macrophage::update_idle() {
	p_state.actual = MacrophageState::idle;
	return MACROPHAGE_BIND(update_idle);
}

bool Macrophage::change_state(MacrophageState next, anim::Parameters params) {
	if (p_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
