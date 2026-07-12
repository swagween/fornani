
#include <fornani/entities/enemy/catalog/Macrophage.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::enemy {

Macrophage::Macrophage(automa::ServiceProvider& svc, world::Map& map, int variant)
	: Enemy(svc, map, "macrophage"), Animatable{svc, "enemy_macrophage", {32, 32}}, m_services(&svc), m_map(&map),
	  m_body{std::make_unique<vfx::Chain>(svc, variant == 0 ? "macrophage_wall" : "epithelioid_wall", sf::Vector2i{14, 14}, vfx::SpringParameters{0.999f, 0.08f, 16.5f, 4.f}, get_collider().get_center(), variant == 0 ? 32 : 20, false, 16.5f,
										  true)},
	  m_variant{static_cast<MacrophageVariant>(variant)} {
	NANI_LOG_DEBUG(m_logger, "Size: {}", m_body->links.size());
	p_animations = {{"idle", {0, 4, 40, -1}}};
	animation.set_params(get_params("idle"));
	get_collider().physics.set_friction_componentwise({0.999f, 0.999f});
	Enemy::get_collider().set_flag(shape::ColliderFlags::simple);
	attributes.team = arms::Team::beast;
	m_grab.set_constant_radius(20.f);
	flags.state.set(StateFlags::vulnerable);
	m_grab.sensor.activate();
	m_body->set_free(true);
	m_body->set_num_angles(6);
	if (m_variant == MacrophageVariant::epithelioid) {
		m_grab.set_constant_radius(28.f);
		m_body->set_num_angles(4);
		flags.general.set(GeneralFlags::gravity);
		flags.general.set(GeneralFlags::hurt_on_contact);
	}
}

void Macrophage::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	Enemy::update(svc, map, player);

	// if (m_variant == MacrophageVariant::epithelioid) { get_collider().physics.apply_force({0.f, 0.01f}); }
	get_collider().physics.apply_force(m_body->get_recoil_force() * 0.02f);
	if (is_alert() || player.is_stunned()) {
		auto force = std::lerp(0.002f, 0.006f, 1.f - m_body->get_percentage_colliding());
		if (m_flags.test(MacrophageFlags::caught_player)) { force = 0.007f; }
		if (player.is_stunned()) { force = 0.0075f; }
		m_steering.thrust_seek(get_collider().physics, player.get_collider().get_center(), ThrustParameters{force, .118f, .991f, 60.f});
	} else {
		m_steering.smooth_random_walk(Enemy::get_collider().physics, 0.0005f, 64.f);
	}
	player.is_stunned() ? m_body->flags.set(vfx::ChainFlags::no_collision) : m_body->flags.reset(vfx::ChainFlags::no_collision);
	// handle body visuals
	m_body->set_position(get_collider().get_center());
	m_body->update(svc, map, player);
	if (m_body->sprite) {
		for (auto& link : m_body->links) {
			float max_distance = died() ? 800.f : 260.f;
			float distance = (player.get_collider().get_center() - link.get_bob()).length();
			float proximity = std::clamp(distance / max_distance, 0.f, 1.f);
			link.set_channel(static_cast<int>(std::lerp(0.f, 5.f, proximity)));
		}
	}

	if (died()) {
		if (!m_flags.test(MacrophageFlags::broken)) { m_body->break_all(); }
		m_flags.set(MacrophageFlags::broken);
		flags.general.reset(GeneralFlags::gravity);
		return;
	}
	if (!player.is_stunned()) { get_collider().physics.velocity += map.compute_mtv(get_collider().get_center()) * 0.05f; }
	if (m_body->contains_point(player.get_collider().get_center())) {
		if (!m_flags.test(MacrophageFlags::caught_player)) { svc.soundboard.play_sound("macrophage_absorb"); }
		m_flags.set(MacrophageFlags::caught_player);
		player.set_flag(player::PlayerFlags::in_goo);
	} else {
		m_flags.reset(MacrophageFlags::caught_player);
	}
	m_grab.enable();
	if (m_variant == MacrophageVariant::monocyte) {
		if (m_grab.kill_player(player, player::PlayerDeathType::swallowed, true)) { m_flags.set(MacrophageFlags::swallowed_player); }
	}

	hurt_effect.update();
	if (flags.state.test(StateFlags::hurt)) {
		hurt_effect.start();
		if (sound.hurt_sound_cooldown.is_complete()) {
			if (m_variant == MacrophageVariant::monocyte) { svc.soundboard.play_sound("hit_low"); }
		}
		flags.state.reset(StateFlags::hurt);
		sound.hurt_sound_cooldown.start();
	}

	m_grab.set_position(get_collider().get_center());

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
